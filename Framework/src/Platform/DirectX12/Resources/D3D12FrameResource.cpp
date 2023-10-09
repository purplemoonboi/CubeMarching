#include "Framework/cmpch.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"

namespace Engine
{
	D3D12FrameResource::~D3D12FrameResource()
	{
		
	}

	

	D3D12FrameResource::D3D12FrameResource(GraphicsContext* graphicsContext, 
	                                       UINT passCount, 
	                                       UINT materialBufferCount, 
	                                       UINT objectCount,
	                                       UINT voxelBufferElementCount

	)
	{
		const auto d3d12Context = dynamic_cast<D3D12Context*>(graphicsContext);

		
		/**
		 * Create a command allocator for this resource.
		 */
		HRESULT hr = d3d12Context->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(CommandAlloc.GetAddressOf()));
		THROW_ON_FAILURE(hr);

		/**
		 * Create a graphics command list for this resource.
		 */
		hr = d3d12Context->Device->CreateCommandList(0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			CommandAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(GraphicsCommandList.GetAddressOf()));
		THROW_ON_FAILURE(hr);

		/*hr = CommandAlloc->Reset();
		THROW_ON_FAILURE(hr);*/
		hr = GraphicsCommandList->Close();
		THROW_ON_FAILURE(hr);

		/**
		 * Create buffers for this resource
		 */
		PassBuffer		= CreateScope<D3D12UploadBuffer<PassConstants>>(d3d12Context, passCount, true);
		MaterialBuffer  = CreateScope<D3D12UploadBuffer<MaterialConstants>>(d3d12Context, materialBufferCount, true);
		ConstantBuffer	= CreateScope<D3D12UploadBuffer<ObjectConstant>>(d3d12Context, objectCount, true);
		TerrainBuffer   = CreateScope<D3D12UploadBuffer<Vertex>>(d3d12Context,voxelBufferElementCount, false);

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
