#include "MarchingCubesHP.h"

#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
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
		MemManager = dynamic_cast<D3D12MemoryManager*>(memManager);

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

		const ShaderArgs genTriangles =
		{
			L"assets\\shaders\\ImprovedMarchingCubes.hlsl",
			"GenerateTriangles",
			"cs_5_0"
		};
		GenerateTrianglesCS = Shader::Create(genTriangles.FilePath, genTriangles.EntryPoint, genTriangles.ShaderModel);
		GenerateTrianglesPso = PipelineStateObject::Create(ComputeContext, GenerateTrianglesCS.get(), RootSignature);


		const HRESULT deviceRemovedReason = ComputeContext->Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);
	}

	void MarchingCubesHP::Polygonise(Texture* texture)
	{
		ComputeContext->Wait(&FenceValue);
		ComputeContext->ResetComputeCommandList(GenerateVerticesPso.get());

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetShaderResourceDescHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		auto const genVertsPso = dynamic_cast<D3D12PipelineStateObject*>(GenerateVerticesPso.get());
		ComputeContext->CommandList->SetPipelineState(genVertsPso->GetPipelineState());
		ComputeContext->CommandList->SetComputeRootSignature(RootSignature.Get());

		auto const d3d12Texture = dynamic_cast<D3D12Texture*>(texture);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(1, d3d12Texture->GpuHandleSrv);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(2, LookUpTableSrv);


		ComputeContext->CommandList->SetComputeRootDescriptorTable(3, VertexUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(4, FaceUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(5, IndexUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(6, VertexCounterUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(7, HashMapUav);

		const UINT groupDispatch = (ChunkWidth * ChunkHeight * ChunkWidth) / 512;
		ComputeContext->CommandList->Dispatch(groupDispatch, 1, 1);


		auto const genFacePso = dynamic_cast<D3D12PipelineStateObject*>(GenerateTrianglesPso.get());
		ComputeContext->CommandList->SetPipelineState(genFacePso->GetPipelineState());

		ComputeContext->CommandList->Dispatch(groupDispatch, 1, 1);

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
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(VertexReadBackBuffer.Get(), VertexBuffer.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


		/**
		 *		copy the index buffer back onto CPU
		 */
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(IndicesBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(IndicesReadBackBuffer.Get(), IndicesBuffer.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(IndicesBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


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
		INT32* indexData = nullptr;
		INT32* counterData = nullptr;
		INT32 i = 0;

		std::vector<MCIFace> faces;
		std::vector<MCIVertex> vertices;
		std::vector<MCIEdgeElementTable> hashTable;
		std::vector<INT32> indices;
		std::vector<INT32> counts;

		
		const HRESULT fdMap = FaceReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&faceData));
		THROW_ON_FAILURE(fdMap);
		for(i = 0; i < MarchingCubesNumberOfTriangles; ++i)
		{
			faces.push_back(faceData[i]);
		}
		FaceReadBackBuffer->Unmap(0, nullptr);


		const HRESULT vdMap = VertexReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
		THROW_ON_FAILURE(vdMap);
		for (i = 0; i < MarchingCubesNumberOfVertices; ++i)
		{
			vertices.push_back(vertexData[i]);
		}
		VertexReadBackBuffer->Unmap(0, nullptr);


		const HRESULT idMap = IndicesReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
		THROW_ON_FAILURE(idMap);
		for (i = 0; i < MarchingCubesNumberOfVertices; ++i)
		{
			indices.push_back(indexData[i]);
		}
		IndicesReadBackBuffer->Unmap(0, nullptr);


		const HRESULT cMap  = VertexCounterReadBack->Map(0, nullptr, reinterpret_cast<void**>(&counterData));
		THROW_ON_FAILURE(cMap);
		for (i = 0; i < MarchingCubesNumberOfVertices; ++i)
		{
			counts.push_back(counterData[i]);
		}
		VertexCounterReadBack->Unmap(0, nullptr);


		const HRESULT hmMap = HashMapReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&hashTableData));
		THROW_ON_FAILURE(hmMap);
		for (i = 0; i < MCIHashTableSize32; ++i)
		{
			hashTable.push_back(hashTableData[i]);
		}
		HashMapReadBackBuffer->Unmap(0, nullptr);


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
		CD3DX12_DESCRIPTOR_RANGE indexBuffer;
		indexBuffer.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		CD3DX12_DESCRIPTOR_RANGE vertexCounter;
		vertexCounter.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);
		CD3DX12_DESCRIPTOR_RANGE hashMap;
		hashMap.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 4);


		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[8];
		slotRootParameter[0].InitAsConstants(9, 0);
		slotRootParameter[1].InitAsDescriptorTable(1, &densityTexture);
		slotRootParameter[2].InitAsDescriptorTable(1, &lookupTable);
		slotRootParameter[3].InitAsDescriptorTable(1, &vertexBuffer);
		slotRootParameter[4].InitAsDescriptorTable(1, &faceBuffer);
		slotRootParameter[5].InitAsDescriptorTable(1, &indexBuffer);
		slotRootParameter[6].InitAsDescriptorTable(1, &vertexCounter);
		slotRootParameter[7].InitAsDescriptorTable(1, &hashMap);

		// A root signature is an array of root parameters.
		const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(8, slotRootParameter,
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
		constexpr UINT64 faceCapacity = MarchingCubesNumberOfTriangles * sizeof(MCIFace);
		FaceBuffer = D3D12BufferUtils::CreateStructuredBuffer(faceCapacity, true, true);
		FaceReadBackBuffer = D3D12BufferUtils::CreateReadBackBuffer(faceCapacity);

		constexpr UINT64 vertexCapcity = MarchingCubesNumberOfVertices * sizeof(MCIVertex);
		VertexBuffer = D3D12BufferUtils::CreateStructuredBuffer(vertexCapcity, true, true);
		VertexReadBackBuffer = D3D12BufferUtils::CreateReadBackBuffer(vertexCapcity);

		constexpr UINT64 indexCapacity = MarchingCubesNumberOfVertices * sizeof(INT32);
		IndicesBuffer = D3D12BufferUtils::CreateStructuredBuffer(indexCapacity, true, true);
		IndicesReadBackBuffer = D3D12BufferUtils::CreateReadBackBuffer(indexCapacity);

		constexpr UINT64 counterCapacity = MarchingCubesNumberOfVertices * sizeof(INT32);
		VertexCounter = D3D12BufferUtils::CreateStructuredBuffer(counterCapacity, true, true);
		VertexCounterReadBack = D3D12BufferUtils::CreateReadBackBuffer(counterCapacity);
		D3D12BufferUtils::CreateUploadBuffer(VertexCounterUpload, counterCapacity);

		constexpr UINT64 hashMapCapacity = sizeof(MCIEdgeElementTable);
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

		constexpr auto bufferWidth = (4096 * sizeof(INT32));

		LookUpTableResource = D3D12BufferUtils::CreateDefaultBuffer
		(
			TriangleTable,
			bufferWidth,
			LookUpTableUpload
		);
		
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

		/** create views for the index buffer  */
		uavDesc.Buffer.StructureByteStride = sizeof(INT32);
		uavDesc.Buffer.NumElements = MarchingCubesNumberOfVertices;
		IndexUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, IndicesBuffer.Get());
		/** similar desc to the index view */
		VertexCounterUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, VertexCounter.Get());


		uavDesc.Buffer.StructureByteStride = MCIHashTableSize32 * sizeof(MCIEdgeElementTable);
		uavDesc.Buffer.NumElements = MCIHashTableSize32;
		HashMapUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, HashMapBuffer.Get());

	}
}
