#include "Framework/cmpch.h"
#include "Platform/DirectX12/Resources/D3D12RenderFrame.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Foundation::Graphics::D3D12
{
	D3D12RenderFrame::~D3D12RenderFrame()
	{
		
	}


	D3D12RenderFrame::D3D12RenderFrame()
		:
	pGCL(nullptr),
	pCmdAlloc(nullptr)
	{
	}

	D3D12RenderFrame::D3D12RenderFrame(UINT passCount, UINT objectCount)
	{

		
		/**
		 * Create a command allocator for this resource.
		 */
		HRESULT hr = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(pCmdAlloc.GetAddressOf()));
		THROW_ON_FAILURE(hr);

		/**
		 * Create a graphics command list for this resource.
		 */
		hr = pDevice->CreateCommandList(0,
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
		PassBuffer		= D3D12UploadBuffer<PassConstants>(passCount, true);
		ConstantBuffer	= D3D12UploadBuffer<ObjectConstant>(objectCount, true);

		INT32 i;
		for (i = 0; i < GBUFFER_SIZE; ++i)
		{
			RenderFrames[i] = CreateScope<D3D12RenderTarget>(nullptr, 1920, 1080);
		}

	}

	


}
