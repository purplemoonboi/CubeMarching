#include "Framework/cmpch.h"
#include "Platform/DirectX12/DX12FrameResource.h"

namespace Engine
{
	DX12FrameResource::~DX12FrameResource()
	{

	}

	DX12FrameResource::DX12FrameResource(GraphicsContext* graphicsContext, UINT passCount, UINT objectCount)
	{
		const auto dx12DeviceContext = dynamic_cast<DX12GraphicsContext*>(graphicsContext);
		THROW_ON_FAILURE
		(
			dx12DeviceContext->Device->CreateCommandAllocator
			(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(CmdListAlloc.GetAddressOf()
				)
			)
		);

		PassBuffer = CreateScope<UploadBuffer<PassConstants>>(dx12DeviceContext->Device, passCount, true);
		ConstantBuffer = CreateScope<UploadBuffer<ObjectConstant>>(dx12DeviceContext->Device, objectCount, true);
	}

}
