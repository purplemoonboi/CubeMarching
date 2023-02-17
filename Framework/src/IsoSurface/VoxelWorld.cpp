#include "VoxelWorld.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include <../vendor/Microsoft/DDSTextureLoader.h>



namespace Engine
{


	bool VoxelWorld::Init(ComputeApi* context, MemoryManager* memManager, ShaderArgs args)
	{
		ComputeContext = dynamic_cast<D3D12ComputeApi*>(context);

		MemManager = dynamic_cast<D3D12MemoryManager*>(memManager);

		ComputeShader = Shader::Create(args.FilePath, args.EntryPoint, args.ShaderModel);

		BuildComputeRootSignature();
		BuildPso();
		CreateOutputBuffer();
		CreateReadBackBuffer();

		CreateStructuredBuffer();

		return true;
	}

	void VoxelWorld::Dispatch(VoxelWorldSettings const& worldSettings, DirectX::XMFLOAT3 chunkID, Texture* texture)
	{
		ComputeContext->ResetComputeCommandList(nullptr);

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetDescriptorHeap() };
		ComputeContext->ComputeCommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);
		ComputeContext->ComputeCommandList->SetPipelineState(ComputeState.Get());

		//Bind compute shader buffers
		ComputeContext->ComputeCommandList->SetComputeRootSignature(ComputeRootSignature.Get());

		ComputeContext->ComputeCommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.IsoValue, 0);
		ComputeContext->ComputeCommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.TextureSize, 1);
		ComputeContext->ComputeCommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.PlanetRadius, 2);
		ComputeContext->ComputeCommandList->SetComputeRoot32BitConstants(0, 1, &worldSettings.NumOfPointsPerAxis, 3);

		const float coord[3] = {worldSettings.ChunkCoord.x, worldSettings.ChunkCoord.y, worldSettings.ChunkCoord.z};
		ComputeContext->ComputeCommandList->SetComputeRoot32BitConstants(0, 3, coord, 4);

		ComputeContext->ComputeCommandList->SetComputeRootDescriptorTable(1, dynamic_cast<D3D12Texture*>(texture)->GpuHandleSrv);
		ComputeContext->ComputeCommandList->SetComputeRootShaderResourceView(2, TriangleBuffer->GetGPUVirtualAddress());
		ComputeContext->ComputeCommandList->SetComputeRootDescriptorTable(3, OutputVertexUavGpu);

		ComputeContext->ComputeCommandList->Dispatch(ChunkWidth, ChunkHeight, ChunkWidth);


		ComputeContext->ComputeCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->ComputeCommandList->CopyResource(ReadBackBuffer.Get(), OutputBuffer.Get());

		ComputeContext->ComputeCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


		ComputeContext->ComputeCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CounterResource.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->ComputeCommandList->CopyResource(CounterReadback.Get(), CounterResource.Get());

		ComputeContext->ComputeCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CounterResource.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		const INT32 rawData[1] = { 0 };
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = rawData;
		subResourceData.RowPitch = sizeof(INT32);
		subResourceData.SlicePitch = subResourceData.RowPitch;

		ComputeContext->ComputeCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
				CounterResource.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_COPY_DEST
		));

		// Copy the data into the upload heap
		UpdateSubresources(ComputeContext->ComputeCommandList.Get(), CounterResource.Get(), CounterUpload.Get(), 0, 0, 1, &subResourceData);

		// Add the instruction to transition back to read 
		ComputeContext->ComputeCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
				CounterResource.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS
			));

		ComputeContext->ExecuteComputeCommandList();

		INT32* count = nullptr;

		const HRESULT	countMapResult = CounterReadback->Map(0, nullptr, reinterpret_cast<void**>(&count));
		THROW_ON_FAILURE(countMapResult);

		INT32 debug = *count;
		CounterReadback->Unmap(0, nullptr);

		Triangle* data;
		const HRESULT mappingResult = ReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&data));
		THROW_ON_FAILURE(mappingResult);

		RawTriBuffer.clear();
		RawTriBuffer.reserve(*count);
		for(INT32 i = 0; i < *count; ++i)
		{
			RawTriBuffer.push_back(data[i]);
		}
		ReadBackBuffer->Unmap(0, nullptr);

		if(!TerrainMeshGeometry)
		{
			CreateVertexBuffers();
		}
	}


	void VoxelWorld::BuildComputeRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE table0;
		table0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		
		CD3DX12_DESCRIPTOR_RANGE table1;
		table1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

		CD3DX12_DESCRIPTOR_RANGE table2;
		table2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[4];
		slotRootParameter[0].InitAsConstants(5, 0);					// world settings view
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
		HRESULT hr = D3D12SerializeRootSignature
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

	void VoxelWorld::BuildPso()
	{
		auto const d3dCsShader = dynamic_cast<D3D12Shader*>(ComputeShader.get());
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = ComputeRootSignature.Get();
		desc.CS =
		{
			reinterpret_cast<BYTE*>(d3dCsShader->GetShader()->GetBufferPointer()),
			d3dCsShader->GetShader()->GetBufferSize()
		};
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		const HRESULT csPipelineState = ComputeContext->Context->Device->CreateComputePipelineState
		(
			&desc, 
			IID_PPV_ARGS(&ComputeState)
		);
		THROW_ON_FAILURE(csPipelineState);
	}

	#include "Framework/Maths/Perlin.h"


	void VoxelWorld::CreateOutputBuffer()
	{

		constexpr auto bufferWidth = (NumberOfBufferElements * sizeof(Triangle));
		const HRESULT vertexResult = ComputeContext->Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS,
			//D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&OutputBuffer)
		);
		THROW_ON_FAILURE(vertexResult);

		constexpr UINT64 sizeInBytes = sizeof(UINT32);

		const HRESULT counterResult = ComputeContext->Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS,
			//D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&CounterResource)
		);
		THROW_ON_FAILURE(counterResult);

		/** create views for the vertex buffer */
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.StructureByteStride = sizeof(Triangle);
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = NumberOfBufferElements;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		auto handle = MemManager->GetResourceHandle();
		OutputVertexUavGpu = handle.GpuCurrentHandle;
		/* create the view describing our output buffer. note to offset on the GPU address */
		ComputeContext->Context->Device->CreateUnorderedAccessView(
			OutputBuffer.Get(),
			CounterResource.Get(),
			&uavDesc,
			handle.CpuCurrentHandle
		);

		

		const HRESULT deviceRemovedReasonUav = ComputeContext->Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReasonUav);
	}

	void VoxelWorld::CreateReadBackBuffer()
	{
		constexpr auto bufferWidth = (NumberOfBufferElements * sizeof(Triangle));

		const HRESULT readBackResult = ComputeContext->Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&ReadBackBuffer)
		);

		THROW_ON_FAILURE(readBackResult);

		const HRESULT countReadBackResult = ComputeContext->Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(4),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&CounterReadback)
		);
		THROW_ON_FAILURE(countReadBackResult);

		const HRESULT uploadBuffer = ComputeContext->Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(4),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(CounterUpload.GetAddressOf())
		);
		THROW_ON_FAILURE(uploadBuffer);


	}

	void VoxelWorld::CreateStructuredBuffer()
	{
		constexpr auto bufferWidth = (4096 * sizeof(INT32));


		TriangleBuffer = D3D12BufferUtils::CreateVertexBuffer(
			ComputeContext->Context->Device.Get(),
			ComputeContext->Context->GraphicsCmdList.Get(),
			TriangleTable,
			bufferWidth,
			UploadTriBuffer
		);
	}

	void VoxelWorld::CreateVertexBuffers()
	{
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

	}
}

