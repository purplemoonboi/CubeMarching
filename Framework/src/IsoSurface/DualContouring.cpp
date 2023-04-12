#include "DualContouring.h"

#include "Framework/Renderer/Resources/Material.h"
#include "Framework/Renderer/Resources/Shader.h"

#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"

namespace Engine
{
	void DualContouring::Init(ComputeApi* compute, MemoryManager* memManger)
	{
		ComputeContext = dynamic_cast<D3D12ComputeApi*>(compute);
		MemManager = dynamic_cast<D3D12MemoryManager*>(memManger);


		/**
		 * load and compile the shader passes
		 */
		const ShaderArgs argsGVs =
		{
			L"assets\\shaders\\DualContouring.hlsl",
			"GenerateVertices",
			"cs_5_0"
		};
		GenerateVerticesShader = Shader::Create(argsGVs.FilePath, argsGVs.EntryPoint, argsGVs.ShaderModel);

		const ShaderArgs argsTs =
		{
			L"assets\\shaders\\DualContouring.hlsl",
			"GenerateTriangle",
			"cs_5_0"
		};
		GenerateTriangleShader = Shader::Create(argsTs.FilePath, argsTs.EntryPoint, argsTs.ShaderModel);

		
		BuildRootSignature();
		BuildDualContourPipelineStates();
		CreateBuffers();

		Vertices.reserve(VoxelWorldElementCount);
		Vertex vert = {};
		Vertices.insert(Vertices.begin(), VoxelWorldElementCount, vert);
		Indices.reserve(VoxelWorldElementCount);
		Indices.insert(Indices.begin(), VoxelWorldElementCount, 0);

	}

	void DualContouring::Dispatch(const VoxelWorldSettings& settings, Texture* texture)
	{

		//ComputeContext->Wait(&FenceValue);

		ComputeContext->ResetComputeCommandList(GenerateVerticesPso.get());

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetShaderResourceDescHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		auto gts = dynamic_cast<D3D12PipelineStateObject*>(GenerateVerticesPso.get());
		ComputeContext->CommandList->SetPipelineState(gts->GetPipelineState());

		UINT groupXZ = ChunkWidth/8;
		UINT groupY =  ChunkHeight/8;

		/* first pass - compute the vertex positions of each voxel*/
		const auto tex = dynamic_cast<D3D12Texture*>(texture);

		ComputeContext->CommandList->SetComputeRootSignature(RootSignature.Get());

		const float ccoord[] = { settings.ChunkCoord.x, settings.ChunkCoord.y, settings.ChunkCoord.z };
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.IsoValue, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.TextureSize, 1);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.UseBinarySearch, 2);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.NumOfPointsPerAxis, 3);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 3, &ccoord, 4);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.Resolution, 7);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.UseTexture, 8);


		ComputeContext->CommandList->SetComputeRootDescriptorTable(1, tex->GpuHandleSrv);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(2, VertexBufferUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(3, TriangleBufferUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(4, VoxelMatBufferUav);

		ComputeContext->CommandList->Dispatch(groupXZ, groupY, groupXZ);

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(VertexBackBuffer.Get(), VertexBuffer.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexCounterBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(VertexCounterReadBack.Get(), VertexCounterBuffer.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexCounterBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


		/* second pass - generating triangles */

		gts = dynamic_cast<D3D12PipelineStateObject*>(GenerateTrianglePso.get());
		ComputeContext->CommandList->SetPipelineState(gts->GetPipelineState());

		ComputeContext->CommandList->Dispatch(ChunkWidth, ChunkWidth, ChunkWidth);

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(TriangleBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(TriangleReadBackBuffer.Get(), TriangleBuffer.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(TriangleBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(TriangleCounterBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(TriangleCounterReadBack.Get(), TriangleCounterBuffer.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(TriangleCounterBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		/* flush the instructions */
		ComputeContext->FlushComputeQueue(&FenceValue);

		/* map counter values and cache before instructed counter resets */
		//INT32* vcountData = nullptr;
		//INT32 vcounter = 0;
		//const HRESULT vcounterHR = VertexCounterReadBack->Map(0, nullptr, reinterpret_cast<void**>(&vcountData));
		//THROW_ON_FAILURE(vcounterHR);
		//vcounter = *vcountData;
		//VertexCounterReadBack->Unmap(0, nullptr);

		UINT64* countData = nullptr;
		UINT64 counter = 0;

		const HRESULT counterHR = TriangleCounterReadBack->Map(0, nullptr, reinterpret_cast<void**>(&countData));
		THROW_ON_FAILURE(counterHR);

		const HRESULT hh = ComputeContext->Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(hh);

		counter = *countData;

		TriangleCounterReadBack->Unmap(0, nullptr);

		ResetCounters();

		/**
		 * read vertices
		 */
		/*DualContourVertex* rawVerts = nullptr;

		const HRESULT vertHR = VertexBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&rawVerts));
		THROW_ON_FAILURE(vertHR);
		std::vector<DualContourVertex> vertices;

		for (INT32 i = 0; i < DualContourNumberOfElements; ++i)
		{
			vertices.push_back(rawVerts[i]);
		}
		VertexBackBuffer->Unmap(0, nullptr);*/

		/**
		 * read triangles 
		 */
		DualContourTriangle* rawTris = nullptr;

		const HRESULT triHR = TriangleReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&rawTris));
		THROW_ON_FAILURE(triHR);

		RawVoxelBuffer.clear();

		for (INT32 i = 0; i < counter; ++i)
		{
			RawVoxelBuffer.push_back(rawTris[i]);
		}
		TriangleReadBackBuffer->Unmap(0, nullptr);

		if (!RawVoxelBuffer.empty())
		{
			Vertices.clear();
			Indices.clear();

			UINT16 index = 0;
			for (auto& tri : RawVoxelBuffer)
			{
				Vertex vertex;
				vertex.Position = tri.VertexC.Position;
				vertex.Normal = tri.VertexC.Normal;
				vertex.Tangent = tri.VertexC.Tangent;
				Vertices.push_back(vertex);
				Indices.push_back(++index);

				vertex.Position = tri.VertexB.Position;
				vertex.Normal = tri.VertexB.Normal;
				vertex.Tangent = tri.VertexB.Tangent;
				Vertices.push_back(vertex);
				Indices.push_back(++index);

				vertex.Position = tri.VertexA.Position;
				vertex.Normal = tri.VertexA.Normal;
				vertex.Tangent = tri.VertexA.Tangent;
				Vertices.push_back(vertex);
				Indices.push_back(++index);
			}
		}


	}

	void DualContouring::BuildRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE textureSlot;
		textureSlot.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE vertexSlot;
		vertexSlot.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE triangleSlot;
		triangleSlot.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

		CD3DX12_DESCRIPTOR_RANGE materialSlot;
		materialSlot.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);

		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[5];
		slotRootParameter[0].InitAsConstants(9, 0);							// world settings view
		slotRootParameter[1].InitAsDescriptorTable(1, &textureSlot);		// texture 
		slotRootParameter[2].InitAsDescriptorTable(1, &vertexSlot);			// vertex buffer
		slotRootParameter[3].InitAsDescriptorTable(1, &triangleSlot);		// triangle buffer
		slotRootParameter[4].InitAsDescriptorTable(1, &materialSlot);		// triangle buffer


		// A root signature is an array of root parameters.
		const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_NONE
		);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		const HRESULT serializedResult = D3D12SerializeRootSignature
		(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
		}

		THROW_ON_FAILURE(serializedResult);
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

	void DualContouring::BuildDualContourPipelineStates()
	{
		GenerateVerticesPso = PipelineStateObject::Create(ComputeContext, GenerateVerticesShader.get(), RootSignature);
		GenerateTrianglePso = PipelineStateObject::Create(ComputeContext, GenerateTriangleShader.get(), RootSignature);

	}

	void DualContouring::CreateBuffers()
	{
		/* create views for the vertex buffer */
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		/* create the vertex buffer */
		uavDesc.Buffer.StructureByteStride = sizeof(DualContourVertex);
		uavDesc.Buffer.NumElements = DualContourNumberOfElements;
		const UINT64 vertBufferWidth = DualContourNumberOfElements * sizeof(DualContourVertex);

		VertexBuffer = D3D12BufferUtils::CreateStructuredBuffer(vertBufferWidth, true, true);
		VertexBackBuffer = D3D12BufferUtils::CreateReadBackBuffer(vertBufferWidth);

		VertexCounterBuffer = D3D12BufferUtils::CreateCounterResource(true, true);
		VertexCounterReadBack = D3D12BufferUtils::CreateReadBackBuffer(4);
		D3D12BufferUtils::CreateUploadBuffer(VertexCounterUpload, 4);

		VertexBufferUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, VertexBuffer.Get(), 
			VertexCounterBuffer.Get());

		/* create the material buffer */
		uavDesc.Buffer.StructureByteStride = sizeof(INT32);
		uavDesc.Buffer.NumElements = DualContourNumberOfElements;
		const UINT64 matBufferWidth = DualContourNumberOfElements * sizeof(INT32);

		VoxelMaterialBuffer = D3D12BufferUtils::CreateStructuredBuffer(vertBufferWidth, true, true);
		VoxelMatReadBackBuffer = D3D12BufferUtils::CreateReadBackBuffer(vertBufferWidth);

		VoxelMatCounterBuffer = D3D12BufferUtils::CreateCounterResource(true, true);
		VoxelMatCounterReadBack = D3D12BufferUtils::CreateReadBackBuffer(4);
		D3D12BufferUtils::CreateUploadBuffer(VoxelMatCounterUpload, 4);

		VoxelMatBufferUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, VoxelMaterialBuffer.Get(),
			VoxelMatCounterBuffer.Get());

		/* create the buffer to hold the triangles */

		uavDesc.Buffer.StructureByteStride = sizeof(DualContourTriangle);
		uavDesc.Buffer.NumElements = DualContourTriangleNumberOfElements;
		const UINT64 triangleBufferWidth = DualContourTriangleBufferCapacity;

		TriangleBuffer = D3D12BufferUtils::CreateStructuredBuffer(triangleBufferWidth, true, true);
		TriangleReadBackBuffer = D3D12BufferUtils::CreateReadBackBuffer(triangleBufferWidth);

		TriangleCounterBuffer = D3D12BufferUtils::CreateCounterResource(true, true);
		TriangleCounterReadBack = D3D12BufferUtils::CreateReadBackBuffer(4);
		D3D12BufferUtils::CreateUploadBuffer(TriangleCounterUpload, 4);

		TriangleBufferUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, TriangleBuffer.Get(), TriangleCounterBuffer.Get());

	}

	

	void DualContouring::ResetCounters()
	{
		ComputeContext->ResetComputeCommandList(nullptr);

		//const INT32 rawDataA[1] = { 0 };
		//D3D12_SUBRESOURCE_DATA subResourceDataA = {};
		//subResourceDataA.pData = rawDataA;
		//subResourceDataA.RowPitch = sizeof(INT32);
		//subResourceDataA.SlicePitch = subResourceDataA.RowPitch;

		//ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		//	TriangleCounterBuffer.Get(),
		//	D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		//	D3D12_RESOURCE_STATE_COPY_DEST
		//));

		//// Copy the data into the upload heap
		//UpdateSubresources(ComputeContext->CommandList.Get(), VertexCounterBuffer.Get(), VertexCounterUpload.Get(), 0, 0, 1, &subResourceDataA);

		//// Add the instruction to transition back to read 
		//ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		//	TriangleCounterBuffer.Get(),
		//	D3D12_RESOURCE_STATE_COPY_DEST,
		//	D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		//));

		const INT32 rawData[1] = { 0 };
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = rawData;
		subResourceData.RowPitch = sizeof(INT32);
		subResourceData.SlicePitch = subResourceData.RowPitch;

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			TriangleCounterBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COPY_DEST
		));

		// Copy the data into the upload heap
		UpdateSubresources(ComputeContext->CommandList.Get(), TriangleCounterBuffer.Get(), TriangleCounterUpload.Get(), 0, 0, 1, &subResourceData);

		// Add the instruction to transition back to read 
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			TriangleCounterBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		));

		ComputeContext->FlushComputeQueue(&FenceValue);
	}
}
