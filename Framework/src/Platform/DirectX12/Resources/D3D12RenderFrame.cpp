#include "Framework/cmpch.h"
#include "Platform/DirectX12/Resources/D3D12RenderFrame.h"
#include "Platform/DirectX12/D3D12/D3D12.h"

namespace Foundation::Graphics::D3D12
{


	D3D12RenderFrame::~D3D12RenderFrame()
	{
		gD3D12Context->DeferredRelease(PassBuffer.Resource());
		gD3D12Context->DeferredRelease(ConstantBuffer.Resource());


	}


	D3D12RenderFrame::D3D12RenderFrame()
	{
		const auto device = gD3D12Context->GetDevice();

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

	}


}
