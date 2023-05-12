#include "MarchingCubesHP.h"

#include "Platform/DirectX12/Allocator/D3D12HeapManager.h"
#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"

#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"

#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"

#include <random>

namespace Engine
{
	void MarchingCubesHP::Init(ComputeApi* context, MemoryManager* memManager)
	{
		ComputeContext = dynamic_cast<D3D12ComputeApi*>(context);
		MemManager = dynamic_cast<D3D12HeapManager*>(memManager);

		BuildRootSignature();
		BuildResources();
		BuildViews();

		const ShaderArgs genVerts =
		{
			L"assets\\shaders\\ImprovedMarchingCubes.hlsl",
			"GenerateVertices",
			"cs_5_0"
		};
		GenerateVerticesCS = Shader::Create(genVerts.FilePath, genVerts.EntryPoint, genVerts.ShaderModel);
		GenerateVerticesPso = PipelineStateObject::Create(ComputeContext, GenerateVerticesCS.get(), RootSignature);

		const ShaderArgs genIndices =
		{
			L"assets\\shaders\\ImprovedMarchingCubes.hlsl",
			"GenerateIndices",
			"cs_5_0"
		};
		GenerateIndicesCS = Shader::Create(genIndices.FilePath, genIndices.EntryPoint, genIndices.ShaderModel);
		GenerateIndicesPso = PipelineStateObject::Create(ComputeContext, GenerateIndicesCS.get(), RootSignature);

		const ShaderArgs genTriangles =
		{
			L"assets\\shaders\\ImprovedMarchingCubes.hlsl",
			"GenerateFaces",
			"cs_5_0"
		};
		GenerateTrianglesCS = Shader::Create(genTriangles.FilePath, genTriangles.EntryPoint, genTriangles.ShaderModel);
		GenerateTrianglesPso = PipelineStateObject::Create(ComputeContext, GenerateTrianglesCS.get(), RootSignature);

		const ShaderArgs initHashTable =
		{
			L"assets\\shaders\\ImprovedMarchingCubes.hlsl",
			"InitialiseHashTable",
			"cs_5_0"
		};
		InitHashTableCS = Shader::Create(initHashTable.FilePath, initHashTable.EntryPoint, initHashTable.ShaderModel);
		InitHashTablePso = PipelineStateObject::Create(ComputeContext, InitHashTableCS.get(), RootSignature);


		const HRESULT deviceRemovedReason = ComputeContext->Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);

		ComputeContext->ResetComputeCommandList(InitHashTablePso.get());
		ComputeContext->CommandList->SetComputeRootDescriptorTable(6, HashMapUav);
		ComputeContext->CommandList->Dispatch(MCIHashTableSize32, 1, 1);

		/**
		 *	...for debugging
		 */
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(HashMapBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(HashMapReadBackBuffer.Get(), HashMapBuffer.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(HashMapBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


		ComputeContext->FlushComputeQueue(&FenceValue);

		INT32 i = 0;
		const HRESULT hmMap = HashMapReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&HashTableData));
		THROW_ON_FAILURE(hmMap);
		for (i = 0; i < MCIHashTableSize32; ++i)
		{
			HashTableCPU_Copy.push_back(HashTableData[i]);
		}
		HashMapReadBackBuffer->Unmap(0, nullptr);

	}

	void MarchingCubesHP::Polygonise(const VoxelWorldSettings& worldSettings, Texture* texture)
	{
		

		ComputeContext->ResetComputeCommandList(GenerateVerticesPso.get());
		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetShaderResourceDescHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);
		ComputeContext->CommandList->SetComputeRootSignature(RootSignature.Get());


		auto const d3d12Texture = dynamic_cast<D3D12Texture*>(texture);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.IsoValue, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.TextureSize, 1);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.UseBinarySearch, 2);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.NumOfPointsPerAxis, 3);

		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.ChunkCoord.x, 4);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.ChunkCoord.y, 5);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.ChunkCoord.z, 6);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.Resolution, 7);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.UseTexture, 8);

		ComputeContext->CommandList->SetComputeRootDescriptorTable(1, d3d12Texture->GpuHandleSrv);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(2, LookUpTableSrv);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(3, VertexUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(4, FaceUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(5, VertexCounterUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(6, HashMapUav);

		/*...generate vertices and fill out hash table...*/
		UINT groupDispatch = (ChunkWidth * ChunkHeight * ChunkWidth) / 256;
		ComputeContext->CommandList->Dispatch(groupDispatch, 1, 1);


		//auto pso = dynamic_cast<D3D12PipelineStateObject*>(GenerateTrianglesPso.get());
		//ComputeContext->CommandList->SetPipelineState(pso->GetPipelineState());

		//ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.IsoValue, 0);
		//ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.TextureSize, 1);
		//ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.UseBinarySearch, 2);
		//ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.NumOfPointsPerAxis, 3);

		//ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.ChunkCoord.x, 4);
		//ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.ChunkCoord.y, 5);
		//ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.ChunkCoord.z, 6);
		//ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.Resolution, 7);
		//ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.UseTexture, 8);

		//ComputeContext->CommandList->SetComputeRootDescriptorTable(1, d3d12Texture->GpuHandleSrv);
		//ComputeContext->CommandList->SetComputeRootDescriptorTable(2, LookUpTableSrv);
		//ComputeContext->CommandList->SetComputeRootDescriptorTable(3, VertexUav);
		//ComputeContext->CommandList->SetComputeRootDescriptorTable(4, FaceUav);
		//ComputeContext->CommandList->SetComputeRootDescriptorTable(5, VertexCounterUav);
		//ComputeContext->CommandList->SetComputeRootDescriptorTable(6, HashMapUav);

		///*...generate faces and connect geometry...*/
		//ComputeContext->CommandList->Dispatch(groupDispatch, 1, 1);

		/**
		 *		copy the triangle indices back onto CPU
		 */
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(FaceBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(FaceReadBackBuffer.Get(), FaceBuffer.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(FaceBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		/**
		 *		copy the vertex buffer back onto CPU
		 */
		//ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(),
		//	D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		//ComputeContext->CommandList->CopyResource(VertexReadBackBuffer.Get(), VertexBuffer.Get());

		//ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(),
		//	D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));



		/**
		 *		copy the vertex counter buffer back onto CPU
		 */
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexCounter.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(VertexReadBackBuffer.Get(), VertexCounter.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexCounter.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


		/**
		 *		copy the hash table buffer back onto CPU
		 */
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(HashMapBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(HashMapReadBackBuffer.Get(), HashMapBuffer.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(HashMapBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		ComputeContext->FlushComputeQueue(&FenceValue);

		/**
		 *		Map data
		 */
		MCIFace* faceData = nullptr;
		MCIVertex* vertexData = nullptr;
		MCIEdgeElementTable* hashTableData = nullptr;
		INT32* counterData = nullptr;
		INT32 i = 0;

		std::vector<INT32> counts;

		
		/*const HRESULT fdMap = FaceReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&faceData));
		THROW_ON_FAILURE(fdMap);
		for(i = 0; i < MarchingCubesNumberOfTriangles; ++i)
		{
			RawIndexBuffer.push_back(faceData[i].I0);
			RawIndexBuffer.push_back(faceData[i].I1);
			RawIndexBuffer.push_back(faceData[i].I2);
		}
		FaceReadBackBuffer->Unmap(0, nullptr);
		*/
		INT32 masks;
		const HRESULT vdMap = VertexReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&counterData));
		THROW_ON_FAILURE(vdMap);
		for (i = 0; i < MarchingCubesNumberOfVertices; ++i)
		{
			Vertex v;
			v.Position = vertexData[i].Position;
			v.TexCoords.x = vertexData[i].WorldIndex;
			RawVertexBuffer.push_back(v);
		}
		VertexReadBackBuffer->Unmap(0, nullptr);


		const HRESULT cMap  = VertexIndicesReadBack->Map(0, nullptr, reinterpret_cast<void**>(&counterData));
		THROW_ON_FAILURE(cMap);
		for (i = 0; i < (ChunkWidth * ChunkHeight * ChunkWidth) * 3; ++i)
		{
			counts.push_back(counterData[i]);
		}
		VertexIndicesReadBack->Unmap(0, nullptr);

		HashTableCPU_Copy.clear();
		for (i = 0; i < MCIHashTableSize32; ++i)
		{
			HashTableCPU_Copy.push_back(HashTableData[i]);
		}


	}


	void MarchingCubesHP::BuildRootSignature()
	{
		//	SRVs
		CD3DX12_DESCRIPTOR_RANGE densityTexture;
		densityTexture.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE lookupTable;
		lookupTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		CD3DX12_DESCRIPTOR_RANGE vertexBuffer;
		vertexBuffer.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE faceBuffer;
		faceBuffer.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		
		CD3DX12_DESCRIPTOR_RANGE vertexCounter;
		vertexCounter.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		CD3DX12_DESCRIPTOR_RANGE hashMap;
		hashMap.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);


		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[7];
		slotRootParameter[0].InitAsConstants(9, 0);
		slotRootParameter[1].InitAsDescriptorTable(1, &densityTexture);
		slotRootParameter[2].InitAsDescriptorTable(1, &lookupTable);
		slotRootParameter[3].InitAsDescriptorTable(1, &vertexBuffer);
		slotRootParameter[4].InitAsDescriptorTable(1, &faceBuffer);
		slotRootParameter[5].InitAsDescriptorTable(1, &vertexCounter);
		slotRootParameter[6].InitAsDescriptorTable(1, &hashMap);

		// A root signature is an array of root parameters.
		const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(7, slotRootParameter,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_NONE
		);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		const HRESULT hr = D3D12SerializeRootSignature
		(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}

		THROW_ON_FAILURE(hr);
		const HRESULT rootSigResult = ComputeContext->Context->Device->CreateRootSignature
		(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(RootSignature.GetAddressOf()
			)
		);
		THROW_ON_FAILURE(rootSigResult);
	}

	void MarchingCubesHP::BuildResources()
	{
		constexpr auto bufferWidth = (4096 * sizeof(INT32));

		LookUpTableResource = D3D12BufferUtils::CreateDefaultBuffer
		(
			TriangleTable,
			bufferWidth,
			LookUpTableUpload
		);

		constexpr UINT64 faceCapacity = MarchingCubesNumberOfTriangles * sizeof(MCIFace);
		FaceBuffer = D3D12BufferUtils::CreateStructuredBuffer(faceCapacity, true, true);
		FaceReadBackBuffer = D3D12BufferUtils::CreateReadBackBuffer(faceCapacity);

		constexpr UINT64 vertexCapcity = MarchingCubesNumberOfVertices * sizeof(MCIVertex);
		VertexBuffer = D3D12BufferUtils::CreateStructuredBuffer(vertexCapcity, true, true);

		VertexReadBackBuffer = D3D12BufferUtils::CreateReadBackBuffer(vertexCapcity);

		constexpr UINT64 counterCapacity = (ChunkWidth * ChunkHeight * ChunkWidth) * 3 * sizeof(INT32);
		VertexCounter = D3D12BufferUtils::CreateStructuredBuffer(counterCapacity, true, true);
		VertexIndicesReadBack = D3D12BufferUtils::CreateReadBackBuffer(counterCapacity);
		D3D12BufferUtils::CreateUploadBuffer(VertexCounterUpload, counterCapacity);

		constexpr UINT64 hashMapCapacity = MCIHashTableSize32 * sizeof(MCIEdgeElementTable);
		HashMapBuffer = D3D12BufferUtils::CreateStructuredBuffer(hashMapCapacity, true, true);
		HashMapReadBackBuffer = D3D12BufferUtils::CreateReadBackBuffer(hashMapCapacity);
	}


	void MarchingCubesHP::BuildViews()
	{
		/** create view for the look up table */
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.StructureByteStride = sizeof(INT32);
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = 4096;

		
		LookUpTableSrv = D3D12Utils::CreateShaderResourceView(srvDesc,
			LookUpTableResource.Get());

		/** create views for the face buffer */
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		uavDesc.Buffer.StructureByteStride = sizeof(MCIVertex);
		uavDesc.Buffer.NumElements = MarchingCubesNumberOfVertices;
		VertexUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, VertexBuffer.Get());

		/** create views for the vertex buffer  */
		uavDesc.Buffer.StructureByteStride = sizeof(MCIFace);
		uavDesc.Buffer.NumElements = MarchingCubesNumberOfTriangles;
		FaceUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, FaceBuffer.Get());

		/** create views for the counter buffer  */
		uavDesc.Buffer.StructureByteStride = sizeof(INT32);
		uavDesc.Buffer.NumElements = (ChunkWidth * ChunkHeight * ChunkWidth) * 3;
		VertexCounterUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, VertexCounter.Get());


		uavDesc.Buffer.StructureByteStride = sizeof(MCIEdgeElementTable);
		uavDesc.Buffer.NumElements = MCIHashTableSize32;
		HashMapUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, HashMapBuffer.Get());

	}
}
