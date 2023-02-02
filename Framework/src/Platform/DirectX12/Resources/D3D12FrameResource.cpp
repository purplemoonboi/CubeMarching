#include "Framework/cmpch.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"

namespace Engine
{
	D3D12FrameResource::~D3D12FrameResource()
	{

	}

	D3D12FrameResource::D3D12FrameResource(GraphicsContext* graphicsContext, UINT passCount, UINT materialBufferCount, UINT objectCount, UINT id)
	{
		const auto dx12DeviceContext = dynamic_cast<D3D12Context*>(graphicsContext);

		/**
		 * Create a command allocator for this resource
		 */
		THROW_ON_FAILURE
		(
			dx12DeviceContext->Device->CreateCommandAllocator
			(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(CmdListAlloc.GetAddressOf()
				)
			)
		);

		/**
		 * Create buffers for this resource
		 */
		PassBuffer		= CreateScope<D3D12UploadBuffer<PassConstants>>(dx12DeviceContext, passCount, true);
		MaterialBuffer  = CreateScope<D3D12UploadBuffer<MaterialConstants>>(dx12DeviceContext, materialBufferCount, true);
		ConstantBuffer	= CreateScope<D3D12UploadBuffer<ObjectConstant>>(dx12DeviceContext, objectCount, true);
		frId = id;
	}

}
