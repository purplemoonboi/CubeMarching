#include "VoxelWorld.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Buffers/D3D12Buffers.h"

#include <../vendor/Microsoft/DDSTextureLoader.h>

namespace Engine
{
	

	bool VoxelWorld::Init(GraphicsContext* c)
	{
		Context = dynamic_cast<D3D12Context*>(c);
		BuildComputeRootSignature();
		BuildPso();
		BuildResourcesAndViews();
		CreateReadBackBuffer();

		return true;
	}

	MCTriangle* VoxelWorld::GenerateChunk(DirectX::XMFLOAT3 chunkID)
	{
		THROW_ON_FAILURE(Context->CmdListAlloc->Reset());
		THROW_ON_FAILURE(Context->GraphicsCmdList->Reset(Context->CmdListAlloc.Get(), ComputeState.Get()));


		ID3D12DescriptorHeap* srvHeap[] = { SrvUavHeap.Get() };
		Context->GraphicsCmdList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);
		Context->GraphicsCmdList->SetPipelineState(ComputeState.Get());

		// #1 - Bind the compute shader root signature.
		Context->GraphicsCmdList->SetComputeRootSignature(ComputeRootSignature.Get());

		// #2 - Set the compute shader variables.
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0,	 1,		&WorldSettings.IsoValue,			0);
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0,	 1,		&WorldSettings.PlanetRadius,		1);
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0,	 1,		&WorldSettings.TextureSize,		2);
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0,	 1,		&WorldSettings.ChunkCoord,		3);
		float coord[3] = { 0,0,0 };
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0,	 3,		coord,		4);

		// Set the density texture
		Context->GraphicsCmdList->SetComputeRootDescriptorTable(1, ScalarFieldSrvGpu);

		// Set the output buffer.
		Context->GraphicsCmdList->SetComputeRootDescriptorTable(2, OutputVertexUavGpu);

		// #3 - Dispatch the work onto the GPU.
		constexpr UINT32 dimension = 32;
		Context->GraphicsCmdList->Dispatch(dimension, dimension, dimension);


		// #4 - Read back the vertices into the vertex buffer.
		Context->GraphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
		

		// Copy vertices from the compute shader into the vertex buffer for rendering
		Context->GraphicsCmdList->CopyResource(ReadbackBuffer.Get(), OutputBuffer.Get());

		Context->GraphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	
		THROW_ON_FAILURE(Context->GraphicsCmdList->Close());
		ID3D12CommandList* cmdLists[] = { Context->GraphicsCmdList.Get() };
		Context->CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
		Context->FlushCommandQueue();


		THROW_ON_FAILURE(ReadbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&RawTriBuffer)));

		MCVertex v  = RawTriBuffer[0].VertexA;
		MCVertex vb = RawTriBuffer[0].VertexB;
		MCVertex vc = RawTriBuffer[0].VertexC;

		return RawTriBuffer;
	}


	void VoxelWorld::BuildComputeRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE table0;
		table0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE table1;
		table1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[3];
		slotRootParameter[0].InitAsConstants(5, 0); // world settings view
		slotRootParameter[1].InitAsDescriptorTable(1, &table0); // density texture view
		slotRootParameter[2].InitAsDescriptorTable(1, &table1);// output buffer view

		// A root signature is an array of root parameters.
		const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter,
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

	void VoxelWorld::BuildResourcesAndViews()
	{
		/**
		 * create our SRV heap
		 * //TODO: You would create the heap elsewhere and save an offset to the
		 * //TODO: voxel world resources.
		 */
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 2;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		HRESULT heapResult = Context->Device->CreateDescriptorHeap(&srvHeapDesc,
			IID_PPV_ARGS(&SrvUavHeap));
		THROW_ON_FAILURE(heapResult);

		const HRESULT deviceRemovedReasonSrvHeap = Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReasonSrvHeap);

		CreateScalarField();

		CreateOutputBuffer();
	}

	#include "Framework/Maths/Perlin.h"


	void VoxelWorld::CreateScalarField()
	{
		/**
		 * get the offset to our voxel world resources in the heap
		 */
		const UINT32 srvDescriptorSize = Context->CbvSrvUavDescriptorSize;
		/* get a pointer to the beginning of the descriptors in the heap. */
		auto cpuHandleOffset = CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetCPUDescriptorHandleForHeapStart(),
			0, srvDescriptorSize);
		auto gpuHandleOffset = CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetGPUDescriptorHandleForHeapStart(),
			0, srvDescriptorSize);

		/*Fill the raw texture with noise values*/
		RawScalarTexture.reserve(VoxelWorldSize * VoxelWorldSize);
		for (UINT32 i = 0; i < VoxelWorldSize; ++i)
		{
			for (UINT32 j = 0; j < VoxelWorldSize; ++j)
			{
				for (UINT32 k = 0; k < VoxelWorldSize; ++k)
				{
					RawScalarTexture.push_back(Perlin((float)i * 0.01f, (float)j * 0.01f, (float)k * 0.01f));
				}
			}
		}

		/* create the upload buffer and copy the noise data into the buffer. */
		CS_Texture = D3D12BufferUtils::CreateTexture3D
		(
			Context->Device.Get(),
			Context->GraphicsCmdList.Get(),
			VoxelWorldSize,
			VoxelWorldSize,
			VoxelWorldSize,
			RawScalarTexture.data(),
			DXGI_FORMAT_R32_FLOAT,
			CS_Upload_Texture
		);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Texture3D.MipLevels = -1;
		srvDesc.Texture3D.MostDetailedMip = 0;

		Context->Device->CreateShaderResourceView(CS_Texture.Get(), &srvDesc, cpuHandleOffset);
		ScalarFieldSrvGpu = gpuHandleOffset.Offset(1, srvDescriptorSize);

		const HRESULT deviceRemovedReasonSrv = Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReasonSrv);
	}

	void VoxelWorld::CreateOutputBuffer()
	{
		/**
		 * get the offset to our voxel world resources in the heap
		 */
		const UINT32 srvDescriptorSize = Context->CbvSrvUavDescriptorSize;
		/* get a pointer to the beginning of the descriptors in the heap. */
		auto cpuHandleOffset = CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetCPUDescriptorHandleForHeapStart(),
			0, srvDescriptorSize);
		auto gpuHandleOffset = CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetGPUDescriptorHandleForHeapStart(),
			0, srvDescriptorSize);

		constexpr auto bufferWidth = (NumberOfBufferElements * sizeof(MCTriangle));


		const HRESULT vertexResult = Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&OutputBuffer)
		);
		THROW_ON_FAILURE(vertexResult);

		const HRESULT counterResult = Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(4, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
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

		/* create the view describing our output buffer. note to offset on the GPU address */
		Context->Device->CreateUnorderedAccessView(
			OutputBuffer.Get(),
			CounterResource.Get(),
			&uavDesc,
			cpuHandleOffset.Offset(1, srvDescriptorSize)
		);

		OutputVertexUavGpu = gpuHandleOffset.Offset(1, srvDescriptorSize);

		const HRESULT deviceRemovedReasonUav = Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReasonUav);
	}

	void VoxelWorld::CreateReadBackBuffer()
	{
		constexpr auto bufferWidth = (NumberOfBufferElements * sizeof(MCTriangle));

		/*create a readback buffer*/
		const HRESULT readbackResult = Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&ReadbackBuffer)
		);
		THROW_ON_FAILURE(readbackResult);
	}
}

