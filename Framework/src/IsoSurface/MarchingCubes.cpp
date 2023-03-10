#include "MarchingCubes.h"
#include <../vendor/Microsoft/DDSTextureLoader.h>

#include "Framework/Core/Log/Log.h"
#include "Framework/Renderer/Pipeline/PipelineStateObject.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"

#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"

namespace Engine
{


	bool MarchingCubes::Init(ComputeApi* context, MemoryManager* memManager, ShaderArgs args)
	{
		ComputeContext = dynamic_cast<D3D12ComputeApi*>(context);

		MemManager = dynamic_cast<D3D12MemoryManager*>(memManager);

		ComputeShader = Shader::Create(args.FilePath, args.EntryPoint, args.ShaderModel);

		BuildComputeRootSignature();
		BuildPso();
		CreateCounterBuffer();
		CreateOutputBuffer();
		CreateTriangulationTableBuffer();

		const HRESULT deviceRemovedReason = ComputeContext->Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);

		return true;
	}

	void MarchingCubes::Dispatch(VoxelWorldSettings const& worldSettings, DirectX::XMFLOAT3 chunkID, Texture* texture, INT32 X, INT32 Y, INT32 Z)
	{

		//ComputeContext->Wait(&FenceValue);

		//ComputeContext->ResetComputeCommandList(ComputeState.get());

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetShaderResourceDescHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		auto const d3d12Pso = dynamic_cast<D3D12PipelineStateObject*>(ComputeState.get());
		ComputeContext->CommandList->SetPipelineState(d3d12Pso->GetPipelineState());

		//Bind compute shader buffers
		ComputeContext->CommandList->SetComputeRootSignature(ComputeRootSignature.Get());

		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.IsoValue, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.TextureSize, 1);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.PlanetRadius, 2);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.NumOfPointsPerAxis, 3);

		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.ChunkCoord.x, 4);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.ChunkCoord.y, 5);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.ChunkCoord.z, 6);

		auto const d3d12Texture = dynamic_cast<D3D12Texture*>(texture);

		ComputeContext->CommandList->SetComputeRootDescriptorTable(1, d3d12Texture->GpuHandleSrv);
		ComputeContext->CommandList->SetComputeRootShaderResourceView(2, TriangulationTable->GetGPUVirtualAddress());
		ComputeContext->CommandList->SetComputeRootDescriptorTable(3, OutputVertexUavGpu);


		ComputeContext->CommandList->Dispatch(X, Y, Z);


		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(ReadBackBuffer.Get(), OutputBuffer.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CounterResource.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(CounterReadback.Get(), CounterResource.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CounterResource.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		const INT32 rawData[1] = { 0 };
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = rawData;
		subResourceData.RowPitch = sizeof(INT32);
		subResourceData.SlicePitch = subResourceData.RowPitch;

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
				CounterResource.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_COPY_DEST
		));

		// Copy the data into the upload heap
		UpdateSubresources(ComputeContext->CommandList.Get(), CounterResource.Get(), CounterUpload.Get(), 0, 0, 1, &subResourceData);

		// Add the instruction to transition back to read 
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
				CounterResource.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS
			));


		ComputeContext->FlushComputeQueue(&FenceValue);

		INT32* atomicCount = nullptr;

		const HRESULT	countMapResult = CounterReadback->Map(0, nullptr, reinterpret_cast<void**>(&atomicCount));
		THROW_ON_FAILURE(countMapResult);

		INT32 triCount = *atomicCount;
		CounterReadback->Unmap(0, nullptr);


		Triangle* data;
		const HRESULT mappingResult = ReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&data));
		THROW_ON_FAILURE(mappingResult);

		RawTriBuffer.clear();
		RawTriBuffer.reserve(triCount);


		for(INT32 i = 0; i < triCount; ++i)
		{
			if (&data[i] != nullptr)
			{
				RawTriBuffer.push_back(data[i]);
			}
		}
		ReadBackBuffer->Unmap(0, nullptr);

		
		CreateVertexBuffers();
		
	}


	void MarchingCubes::BuildComputeRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE table0;
		table0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		
		CD3DX12_DESCRIPTOR_RANGE table1;
		table1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

		CD3DX12_DESCRIPTOR_RANGE table2;
		table2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[4];
		slotRootParameter[0].InitAsConstants(7, 0);					// world settings view
		slotRootParameter[1].InitAsDescriptorTable(1, &table0);		// density texture buffer 
		slotRootParameter[2].InitAsShaderResourceView(1);			// tri table texture 
		slotRootParameter[3].InitAsDescriptorTable(1, &table2);		// output buffer 

		// A root signature is an array of root parameters.
		const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
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
			IID_PPV_ARGS(ComputeRootSignature.GetAddressOf()
			)
		);
		THROW_ON_FAILURE(rootSigResult);
	}

	void MarchingCubes::BuildPso()
	{
		ComputeState = PipelineStateObject::Create(ComputeContext, ComputeShader.get(), ComputeRootSignature);
	}

	#include "Framework/Maths/Perlin.h"


	void MarchingCubes::CreateOutputBuffer()
	{

		OutputBuffer	= D3D12BufferUtils::CreateStructuredBuffer(sizeof(Triangle) * VoxelWorldElementCount, true, true);
		ReadBackBuffer	= D3D12BufferUtils::CreateReadBackBuffer(sizeof(Triangle) * VoxelWorldElementCount);

		/** create views for the vertex buffer */
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.StructureByteStride = sizeof(Triangle);
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = VoxelWorldElementCount;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		OutputVertexUavGpu = D3D12Utils::CreateUnorderedAccessView(uavDesc, OutputBuffer.Get(), CounterResource.Get());
	}

	void MarchingCubes::CreateCounterBuffer()
	{
		/* create the counter buffer */
		CounterResource = D3D12BufferUtils::CreateCounterResource(true, true);

		/* create the counter read back buffer */
		CounterReadback = D3D12BufferUtils::CreateReadBackBuffer(4);

		/* create the counter upload buffer */
		D3D12BufferUtils::CreateUploadBuffer(CounterUpload, 4);
	}


	void MarchingCubes::CreateTriangulationTableBuffer()
	{
		constexpr auto bufferWidth = (4096 * sizeof(INT32));

		TriangulationTable = D3D12BufferUtils::CreateDefaultBuffer
		(
			TriangleTable,
			bufferWidth,
			UploadTriangulationTable
		);
	}

	void MarchingCubes::CreateVertexBuffers()
	{
		if (RawTriBuffer.empty())
			return;

		const HRESULT allocResult = ComputeContext->Context->Allocator->Reset();
		THROW_ON_FAILURE(allocResult);
		const HRESULT listResult = ComputeContext->Context->CmdList->Reset(ComputeContext->Context->Allocator.Get(), nullptr);
		THROW_ON_FAILURE(listResult);

		std::vector<Vertex> vertices;
		std::vector<UINT16> indices;
		UINT16 index = 0;
		for (auto& tri : RawTriBuffer)
		{
			vertices.push_back(tri.VertexC);
			indices.push_back(++index);
			vertices.push_back(tri.VertexB);
			indices.push_back(++index);
			vertices.push_back(tri.VertexA);
			indices.push_back(++index);
		}

		const UINT ibSizeInBytes = sizeof(UINT16) * indices.size();
		const UINT vbSizeInBytes = sizeof(Vertex) * vertices.size();

		TerrainMeshGeometry = CreateScope<MeshGeometry>("Terrain");
		TerrainMeshGeometry->VertexBuffer = VertexBuffer::Create(ComputeContext->Context, vertices.data(),
			vbSizeInBytes, vertices.size(), true);

		const BufferLayout layout =
		{
			{"POSITION",	ShaderDataType::Float3, 0, 0, false },
			{"NORMAL",		ShaderDataType::Float3, 12,1, false },
			{"TEXCOORD",	ShaderDataType::Float2, 24,2, false },
		};
		TerrainMeshGeometry->VertexBuffer->SetLayout(layout);

		TerrainMeshGeometry->IndexBuffer = IndexBuffer::Create(ComputeContext->Context, indices.data(),
			ibSizeInBytes, indices.size());

		const HRESULT closeResult = ComputeContext->Context->CmdList->Close();
		THROW_ON_FAILURE(closeResult);
		ComputeContext->Context->ExecuteGraphicsCommandList();
		ComputeContext->Context->FlushCommandQueue();

		const HRESULT deviceRemovedReason = ComputeContext->Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);

	}
}

