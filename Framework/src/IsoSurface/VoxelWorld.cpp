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
		BuildResourceViews(nullptr);
		return true;
	}

	void VoxelWorld::GenerateChunk(DirectX::XMFLOAT3 chunkID)
	{

		ID3D12DescriptorHeap* srvHeap[] = { SrvUavHeap.Get() };
		Context->GraphicsCmdList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		// #1 - Bind the compute shader root signature.
		Context->GraphicsCmdList->SetComputeRootSignature(ComputeRootSignature.Get());
		Context->GraphicsCmdList->SetPipelineState(ComputeState.Get());

		// #2 - Set the compute shader variables.
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0,	 1,		&WorldSettings.IsoValue,			0);
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0,	 1,		&WorldSettings.PlanetRadius,		1);
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0,	 1,		&WorldSettings.TextureSize,		2);
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0,	 1,		&WorldSettings.ChunkCoord,		3);
		float coord[3] = { 0,0,0 };
		Context->GraphicsCmdList->SetComputeRoot32BitConstants(0,	 3,		coord,		4);

		// Set the density texture
		Context->GraphicsCmdList->SetComputeRootDescriptorTable(1,
			ScalarFieldSrvGpu);

		// Set the output buffer.
		Context->GraphicsCmdList->SetComputeRootDescriptorTable(2,
			OutputVertexUavGpu);


		// #3 - Dispatch the work onto the GPU.
		constexpr UINT32 dimension = 32;
		Context->GraphicsCmdList->Dispatch(dimension, dimension, dimension);

		// #4 - Read back the vertices into the vertex buffer.
		Context->GraphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CS_OutputVertexResource.Get(),
				D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE));
		

		// Copy vertices from the compute shader into the vertex buffer for rendering
		Context->GraphicsCmdList->CopyResource(CS_Readback_OutputVertexResource.Get(), CS_OutputVertexResource.Get());

		Context->GraphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CS_OutputVertexResource.Get(),
				D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
	

		//THROW_ON_FAILURE(CS_Readback_OutputVertexResource->Map(0, nullptr,
		//	reinterpret_cast<void**>(RawTriBuffer)));

		//const MCTriangle* tris[] = { RawTriBuffer };
		//constexpr INT32 count = _countof(tris);
		//INT32 index = 0;

		//CS_Readback_OutputVertexResource->Unmap(0, nullptr);

	}


	void VoxelWorld::BuildComputeRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE srvTable;
		srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE uavTable;
		uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[3];
		// Perfomance TIP: Order from most frequent to least frequent.
		slotRootParameter[0].InitAsConstants(5, 0); // world settings view
		slotRootParameter[1].InitAsDescriptorTable(1, &srvTable); // density texture view
		slotRootParameter[2].InitAsDescriptorTable(1, &uavTable);// output buffer view

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
		THROW_ON_FAILURE(Context->Device->CreateComputePipelineState
		(
			&desc, 
			IID_PPV_ARGS(&ComputeState)
		));
	}

	void VoxelWorld::BuildResourceViews(ID3D12Resource* pCounterResource)
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

		/**
		 * get the offset to our voxel world resources in the heap
		 */
		const UINT32 srvDescriptorSize = Context->CbvSrvUavDescriptorSize;
		/* get a pointer to the beginning of the descriptors in the heap. */
		auto cpuHandleOffset = CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetCPUDescriptorHandleForHeapStart(), 
			0, srvDescriptorSize);
		auto gpuHandleOffset = CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetGPUDescriptorHandleForHeapStart(),
			0, srvDescriptorSize);


		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Texture3D.MostDetailedMip = 0;
		srvDesc.Texture3D.MipLevels = 1;
		
		Context->Device->CreateShaderResourceView(
			CS_Texture.Get(), 
			&srvDesc, 
			cpuHandleOffset
		);
		ScalarFieldSrvGpu = gpuHandleOffset;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Texture2D.MipSlice = 0;


		/*create the view describing our output buffer. note to offset on the GPU address*/
		Context->Device->CreateUnorderedAccessView(
			CS_OutputVertexResource.Get(),
			nullptr, 
			&uavDesc,
			cpuHandleOffset.Offset(1, srvDescriptorSize)
		);

		OutputVertexUavGpu = gpuHandleOffset.Offset(1, srvDescriptorSize);

		BuildResources();
	}

	#include "Framework/Maths/Perlin.h"


	void VoxelWorld::BuildResources()
	{
		/**
		 * create the append buffer resource
		 */
		const HRESULT vertexResult = Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(VoxelWorldVertexBufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&CS_OutputVertexResource)
		);
		THROW_ON_FAILURE(vertexResult);

		const HRESULT readbackResult = Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(VoxelWorldVertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&CS_Readback_OutputVertexResource)
		);
		THROW_ON_FAILURE(readbackResult);

		/**
		 * create the texture resource
		 */



		/*Fill the raw texture with noise values*/
		RawScalarTexture.reserve(VoxelWorldSize * VoxelWorldSize * VoxelWorldSize);
		for (UINT32 i = 0; i < VoxelWorldSize; ++i)
		{
			for (UINT32 j = 0; j < VoxelWorldSize; ++j)
			{
				for (UINT32 k = 0; k < VoxelWorldSize; ++k)
				{
					RawScalarTexture.push_back(Perlin(
						(float)i * 0.001f, 
						(float)j * 0.001f, 
						(float)k * 0.001f)
					);
				}
			}
		}

		/* create the upload buffer and copy the noise data into the buffer. */
		CS_Texture = D3D12BufferUtils::Create_Texture3D_UploadAndDefaultBuffers
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
	}
}

