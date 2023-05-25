#include "DualContouring.h"

#include "Framework/Renderer/Resources/Material.h"
#include "Framework/Renderer/Resources/Shader.h"

#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"


namespace Foundation
{
	void DualContouring::Init(ComputeApi* compute, MemoryManager* memManger)
	{
		ComputeContext = dynamic_cast<D3D12ComputeApi*>(compute);
		MemManager = dynamic_cast<D3D12HeapManager*>(memManger);

		/**
		 * load and compile the shader passes
		 */
		const ShaderArgs argsGVs =
		{
			L"assets\\shaders\\DualContouring.hlsl",
			"GenerateVertices",
			"cs_5_0"
		};
		GenerateVerticesDualContourShader = Shader::Create(argsGVs.FilePath, argsGVs.EntryPoint, argsGVs.ShaderModel);

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

	}

	void DualContouring::Dispatch(const VoxelWorldSettings& settings, Texture* texture)
	{

		ComputeContext->ResetComputeCommandList(GenerateVerticesPso.get());

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetShaderResourceDescHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		auto gts = dynamic_cast<D3D12PipelineStateObject*>(GenerateVerticesPso.get());
		ComputeContext->CommandList->SetPipelineState(gts->GetPipelineState());

		UINT groupXZ = (texture->GetWidth() - 1)  /8;
		UINT groupY =  (texture->GetHeight() - 1) /8;

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
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.SurfaceNets, 12);


		ComputeContext->CommandList->SetComputeRootDescriptorTable(1, tex->GpuHandleSrv);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(2, VertexBufferUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(3, TriangleBufferUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(4, VoxelLookUpTableUav);

		ComputeContext->CommandList->Dispatch(groupXZ, groupY, groupXZ);

		/* second pass - generating triangles */

		gts = dynamic_cast<D3D12PipelineStateObject*>(GenerateTrianglePso.get());
		ComputeContext->CommandList->SetPipelineState(gts->GetPipelineState());

		UINT groupXZb = (texture->GetWidth() - 1);
		UINT groupYb  = (texture->GetHeight() - 1);

		ComputeContext->CommandList->Dispatch(groupXZb, groupYb, groupXZb);


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

		if (CountData != nullptr)
			TriangleCount = *CountData;

	
		ResetCounters();
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
		slotRootParameter[0].InitAsConstants(13, 0);							// world settings view
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
		const HRESULT rootSigResult = ComputeContext->Context->pDevice->CreateRootSignature
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
		GenerateVerticesPso = PipelineStateObject::Create(ComputeContext, GenerateVerticesDualContourShader.get(), RootSignature);
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
		uavDesc.Buffer.StructureByteStride = sizeof(Vertex);
		uavDesc.Buffer.NumElements = DualContourNumberOfElements;
		const UINT64 vertBufferWidth = DualContourNumberOfElements * sizeof(Vertex);

		VertexBuffer = D3D12BufferUtilities::CreateStructuredBuffer(vertBufferWidth, true, true);
		VertexBackBuffer = D3D12BufferUtilities::CreateReadBackBuffer(vertBufferWidth);

		VertexCounterBuffer = D3D12BufferUtilities::CreateCounterResource(true, true);
		VertexCounterReadBack = D3D12BufferUtilities::CreateReadBackBuffer(4);
		D3D12BufferUtilities::CreateUploadBuffer(VertexCounterUpload, 4);

		VertexBufferUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, VertexBuffer.Get(), 
			VertexCounterBuffer.Get());

		/* create the look up buffer */
		uavDesc.Buffer.StructureByteStride = sizeof(INT32);
		uavDesc.Buffer.NumElements = DualContourNumberOfElements;
		const UINT64 matBufferWidth = DualContourNumberOfElements * sizeof(INT32);

		VoxelLookUpTable = D3D12BufferUtilities::CreateStructuredBuffer(vertBufferWidth, true, true);
		VoxelLookUpReadBack = D3D12BufferUtilities::CreateReadBackBuffer(vertBufferWidth);


		VoxelLookUpTableUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, VoxelLookUpTable.Get(),
			nullptr);

		/* create the buffer to hold the triangles */

		uavDesc.Buffer.StructureByteStride = sizeof(Triangle);
		uavDesc.Buffer.NumElements = DualContourTriangleNumberOfElements;
		const UINT64 triangleBufferWidth = DualContourTriangleBufferCapacity;

		TriangleBuffer = D3D12BufferUtilities::CreateStructuredBuffer(triangleBufferWidth, true, true);
		TriangleReadBackBuffer = D3D12BufferUtilities::CreateReadBackBuffer(triangleBufferWidth);

		TriangleCounterBuffer = D3D12BufferUtilities::CreateCounterResource(true, true);
		TriangleCounterReadBack = D3D12BufferUtilities::CreateReadBackBuffer(4);
		D3D12BufferUtilities::CreateUploadBuffer(TriangleCounterUpload, 4);

		TriangleBufferUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, TriangleBuffer.Get(), TriangleCounterBuffer.Get());

		const HRESULT triHR = TriangleReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&RawTriBuffer));
		THROW_ON_FAILURE(triHR);

		const HRESULT counterHR = TriangleCounterReadBack->Map(0, nullptr, reinterpret_cast<void**>(&CountData));
		THROW_ON_FAILURE(counterHR);

	}

	

	void DualContouring::ResetCounters()
	{
		ComputeContext->ResetComputeCommandList(nullptr);

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
