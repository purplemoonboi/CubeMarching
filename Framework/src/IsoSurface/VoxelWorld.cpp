#include "VoxelWorld.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include <../vendor/Microsoft/DDSTextureLoader.h>



namespace Engine
{


	bool VoxelWorld::Init(GraphicsContext* c, MemoryManager* memManager)
	{
		Context = dynamic_cast<D3D12Context*>(c);
		MemManager = dynamic_cast<D3D12MemoryManager*>(memManager);
		BuildComputeRootSignature();
		BuildPso();
		CreateOutputBuffer();
		CreateReadBackBuffer();
		CreateConstantBuffer();

		return true;
	}

	const std::vector<MCTriangle>& VoxelWorld::GenerateChunk(DirectX::XMFLOAT3 chunkID, Texture* texture)
	{
		THROW_ON_FAILURE(Context->CmdListAlloc->Reset());
		THROW_ON_FAILURE(Context->GraphicsCmdList->Reset(Context->CmdListAlloc.Get(), ComputeState.Get()));


		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetDescriptorHeap() };
		Context->GraphicsCmdList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);
		Context->GraphicsCmdList->SetPipelineState(ComputeState.Get());

		// #1 - Bind the compute shader root signature.
		Context->GraphicsCmdList->SetComputeRootSignature(ComputeRootSignature.Get());

		// #2 - Set the compute shader variables.
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0, 1, &WorldSettings.IsoValue, 0);
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0, 1, &WorldSettings.TextureSize, 1);
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0, 1, &WorldSettings.PlanetRadius, 2);
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0, 1, &WorldSettings.NumOfPointsPerAxis, 3);
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0, 3, &WorldSettings.ChunkCoord, 4);


		Context->GraphicsCmdList->SetComputeRootDescriptorTable(1, ConstantBufferCbv);
		// Set the density texture
		Context->GraphicsCmdList->SetComputeRootDescriptorTable(2, dynamic_cast<D3D12Texture*>(texture)->GpuHandleSrv);

		// Set the output buffer.
		Context->GraphicsCmdList->SetComputeRootDescriptorTable(3, OutputVertexUavGpu);

		// #3 - Dispatch the work onto the GPU.
		Context->GraphicsCmdList->Dispatch(VoxelWorldSize, VoxelWorldSize, VoxelWorldSize);


		// #4 - Read back the vertices into the vertex buffer.
		Context->GraphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));


		// Copy vertices from the compute shader into the vertex buffer for rendering
		Context->GraphicsCmdList->CopyResource(ReadBackBuffer.Get(), OutputBuffer.Get());


		Context->GraphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


		HRESULT cmdCloseResult = Context->GraphicsCmdList->Close();
		THROW_ON_FAILURE(cmdCloseResult);

		ID3D12CommandList* cmdLists[] = { Context->GraphicsCmdList.Get() };
		Context->CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

		Context->FlushCommandQueue();

		MCTriangle* data;
		HRESULT mappingResult = ReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&data));
		THROW_ON_FAILURE(mappingResult);

		INT32 count = 32768;

		RawTriBuffer.clear();
		RawTriBuffer.reserve(count);
		for(INT32 i = 0; i < count; ++i)
		{
			RawTriBuffer.push_back(data[i]);
		}

		ReadBackBuffer->Unmap(0, nullptr);

		return RawTriBuffer;
	}


	void VoxelWorld::BuildComputeRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE table0;
		table0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
		//TODO: 

		CD3DX12_DESCRIPTOR_RANGE table1;
		table1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE table2;
		table2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[4];
		slotRootParameter[0].InitAsConstants(5, 0); // world settings view#

		slotRootParameter[1].InitAsDescriptorTable(1, &table0); // world settings view
		slotRootParameter[2].InitAsDescriptorTable(1, &table1); // density texture view
		slotRootParameter[3].InitAsDescriptorTable(1, &table2);// output buffer view

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
		THROW_ON_FAILURE(Context->Device->CreateRootSignature
		(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(ComputeRootSignature.GetAddressOf()
			)
		));
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
		const HRESULT csPipelineState = Context->Device->CreateComputePipelineState
		(
			&desc, 
			IID_PPV_ARGS(&ComputeState)
		);
		THROW_ON_FAILURE(csPipelineState);
	}

	#include "Framework/Maths/Perlin.h"


	void VoxelWorld::CreateOutputBuffer()
	{

		constexpr auto bufferWidth = (NumberOfBufferElements * sizeof(MCTriangle));


		const HRESULT vertexResult = Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			//D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&OutputBuffer)
		);
		THROW_ON_FAILURE(vertexResult);

		constexpr UINT64 sizeInBytes = sizeof(UINT32);

		const HRESULT counterResult = Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			//D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&CounterResource)
		);
		THROW_ON_FAILURE(counterResult);

		/** create views for the vertex buffer */
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.StructureByteStride = sizeof(MCTriangle);
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = NumberOfBufferElements;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		auto handle = MemManager->GetResourceHandle();
		OutputVertexUavGpu = handle.GpuCurrentHandle;
		/* create the view describing our output buffer. note to offset on the GPU address */
		Context->Device->CreateUnorderedAccessView(
			OutputBuffer.Get(),
			CounterResource.Get(),
			&uavDesc,
			handle.CpuCurrentHandle
		);

		

		const HRESULT deviceRemovedReasonUav = Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReasonUav);
	}

	void VoxelWorld::CreateReadBackBuffer()
	{
		constexpr auto bufferWidth = (NumberOfBufferElements * sizeof(MCTriangle));

		const HRESULT readBackResult = Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&ReadBackBuffer)
		);

		THROW_ON_FAILURE(readBackResult);
	}

	void VoxelWorld::CreateConstantBuffer()
	{
		constexpr UINT triangulationTableSize = 256 * 16;
		TriangulationTable = CreateScope<D3D12UploadBuffer<MCData>>(Context, triangulationTableSize, true);

		const UINT sizeInBytes = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(MCData));

		const auto handle = MemManager->GetResourceHandle();

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = TriangulationTable->Resource()->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = sizeInBytes;
		Context->Device->CreateConstantBufferView(&cbvDesc, handle.CpuCurrentHandle);
		ConstantBufferCbv = handle.GpuCurrentHandle;

		TriangulationTable->CopyData(0, TriTableRawData);

	}
}

