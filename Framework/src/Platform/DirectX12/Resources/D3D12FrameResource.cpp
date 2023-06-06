#include "Framework/cmpch.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"

namespace Foundation::Graphics::D3D12
{
	D3D12FrameResource::~D3D12FrameResource()
	{
		
	}

	

	D3D12FrameResource::D3D12FrameResource(ID3D12Device* device,
	                                       UINT passCount, 
	                                       UINT materialBufferCount, 
	                                       UINT objectCount,
	                                       UINT voxelBufferElementCount

	)
	{

		
		/**
		 * Create a command allocator for this resource.
		 */
		HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(pCmdAlloc.GetAddressOf()));
		THROW_ON_FAILURE(hr);

		/**
		 * Create a graphics command list for this resource.
		 */
		hr = device->CreateCommandList(0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			pCmdAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(pGCL.GetAddressOf()));
		THROW_ON_FAILURE(hr);

		/*hr = pCA->Reset();
		THROW_ON_FAILURE(hr);*/
		hr = pGCL->Close();
		THROW_ON_FAILURE(hr);

		/**
		 * Create buffers for this resource
		 */
		PassBuffer		= CreateScope<D3D12UploadBuffer<PassConstants>>(device, passCount, true);
		MaterialBuffer  = CreateScope<D3D12UploadBuffer<MaterialConstants>>(device, materialBufferCount, true);
		ConstantBuffer	= CreateScope<D3D12UploadBuffer<ObjectConstant>>(device, objectCount, true);
		TerrainBuffer   = CreateScope<D3D12UploadBuffer<Vertex>>(device,voxelBufferElementCount, false);

		//RenderTarget = CreateScope<D3D12RenderTarget>(nullptr, 1920, 1080);
	}

	void D3D12FrameResource::UpdateVoxelBuffer(const D3D12Context*context, UINT elementCount)
	{
		TerrainBuffer->Destroy();
		TerrainBuffer->Create(context, elementCount, false);
	}

	bool D3D12FrameResource::QueryTerrainBuffer(UINT elementCount)
	{
		const auto desc = TerrainBuffer->Resource()->GetDesc();
		return (desc.Width < sizeof(Vertex)*elementCount);
	}


}
