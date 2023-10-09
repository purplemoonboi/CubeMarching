#include "Framework/cmpch.h"
#include "D3D12FrameBuffer.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Utilities/D3D12BufferFactory.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Framework/Core/Log/Log.h"


#include "Platform/DirectX12/D3D12/D3D12.h"

namespace Foundation::Graphics::D3D12
{
	D3D12FrameBuffer::D3D12FrameBuffer(const FrameBufferSpecifications& fBufferSpecs, ResourceFormat format)
		:
		FrameBuffer(fBufferSpecs, format)
	{
		ScissorRect = {};
		ScreenViewport = {};
	}

	D3D12FrameBuffer::~D3D12FrameBuffer()
	{}


	void D3D12FrameBuffer::Bind()
	{
		auto pCommandList = gD3D12Context->GetGraphicsCommandList();
		CORE_ASSERT(pCommandList, "The 'graphics command list' has failed...");

		D3D12_RESOURCE_BARRIER barrier;
		ZeroMemory(&barrier, sizeof(D3D12_RESOURCE_BARRIER));
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = TargetBuffer[BackBufferIndex].Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = 0;
		pCommandList->ResourceBarrier(1, &barrier);

		pCommandList->RSSetViewports(1, &ScreenViewport);
		pCommandList->RSSetScissorRects(1, &ScissorRect);
		pCommandList->OMSetRenderTargets(1, &pRTV[BackBufferIndex].CpuHandle, true, &pDSV.CpuHandle);

	}

	void D3D12FrameBuffer::UnBind()
	{
		auto pCommandList = gD3D12Context->GetGraphicsCommandList();
		CORE_ASSERT(pCommandList, "The 'graphics command list' has failed...");

		D3D12_RESOURCE_BARRIER barrier;
		ZeroMemory(&barrier, sizeof(D3D12_RESOURCE_BARRIER));
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = TargetBuffer[BackBufferIndex].Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.Subresource = 0;
		pCommandList->ResourceBarrier(1, &barrier);
	}

	void D3D12FrameBuffer::OnResizeFrameBuffer(FrameBufferSpecifications& specifications)
	{
		HRESULT hr{ S_OK };

		auto pDevice = gD3D12Context->GetDevice();
		auto pCommandList = gD3D12Context->GetGraphicsCommandList();
		auto pCommandAlloc = gD3D12Context->CurrentRenderFrame()->GetFrameAllocator();
		CORE_ASSERT(pDevice, "The 'D3D device' has failed...");


		FrameSpecs = specifications;

		// Flush before changing any resources.
		gD3D12Context->FlushRenderQueue();

		hr = pCommandList->Reset(pCommandAlloc, nullptr);
		THROW_ON_FAILURE(hr);

		// Release the previous resources we will be recreating.
		for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		{
			TargetBuffer[i].Reset();
		}

		DepthStencilBuffer.Reset();

		// If the frame buffer is bound to a swap chain, then resize the swap chain too!
		if (FrameSpecs.SwapChainTarget)
		{
			auto pSwapChain = gD3D12Context->GetSwapChain();

			// Resize the swap chain.
			hr = pSwapChain->ResizeBuffers
			(
				SWAP_CHAIN_BUFFER_COUNT,
				FrameSpecs.Width, FrameSpecs.Height,
				BackBufferFormat,
				DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
			);
			THROW_ON_FAILURE(hr);


			BackBufferIndex = 0;


			D3D12_RENDER_TARGET_VIEW_DESC desc = {};


			for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
			{
				pSwapChain->GetBuffer(i, IID_PPV_ARGS(&TargetBuffer[i]));

				pDevice->CreateRenderTargetView
				(
					TargetBuffer[i].Get(),
					nullptr,
					pRTV[i].CpuHandle
				);
				TargetBuffer->Get()->SetName(L"Swap Chain");
				THROW_ON_FAILURE(pDevice->GetDeviceRemovedReason());
			}
		}

		auto msaaState = pDevice->GetMSAAQuality();
		auto msaaQuality = GetMSAAQuality();

		// Create the depth/stencil buffer and view.
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = FrameSpecs.Width;
		depthStencilDesc.Height = FrameSpecs.Height;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
		// the depth buffer.  Therefore, because we need to create two views to the same resource:
		//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
		//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
		// we need to create the depth buffer resource with a typeless format.  
		depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthStencilDesc.SampleDesc.Count = msaaState ? 4 : 1;
		depthStencilDesc.SampleDesc.Quality = msaaState ? (msaaQuality - 1) : 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DepthStencilFormat;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		hr = pDevice->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf())
		);
		THROW_ON_FAILURE(hr);

		THROW_ON_FAILURE(pDevice->GetDeviceRemovedReason());

		// Create descriptor to mip level 0 of entire resource using the format of the resource.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;

		pDevice->CreateDepthStencilView
		(
			DepthStencilBuffer.Get(),
			&dsvDesc,
			GetDepthStencilViewCpu()
		);


		// Transition the resource from its initial state to be used as a depth buffer.
		gD3D12Context->pGraphicsCommandList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				DepthStencilBuffer.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_DEPTH_WRITE
			)
		);

		// Execute the resize commands.
		hr = gD3D12Context->pGraphicsCommandList->Close();
		THROW_ON_FAILURE(hr);

		ID3D12CommandList* cmdsLists[] = { gD3D12Context->pGraphicsCommandList.Get() };
		gD3D12Context->pQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// Wait until resize is complete.
		gD3D12Context->FlushRenderQueue();

		// Update the viewport transform to cover the client area.
		ScreenViewport.TopLeftX = 0.0f;
		ScreenViewport.TopLeftY = 0.0f;
		ScreenViewport.Width = static_cast<float>(FrameSpecs.Width);
		ScreenViewport.Height = static_cast<float>(FrameSpecs.Height);
		ScreenViewport.MinDepth = 0.0f;
		ScreenViewport.MaxDepth = 1.0f;

		ScissorRect = { 0, 0, FrameSpecs.Width, FrameSpecs.Height };
	}

	



	UINT64 D3D12FrameBuffer::GetFrameBuffer() const
	{
		return 0;
	}

	ID3D12Resource* D3D12FrameBuffer::CurrentBackBuffer() const
	{
		return TargetBuffer[BackBufferIndex].Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12FrameBuffer::GetCurrentBackBufferViewCpu() const
	{
		return pRTV[BackBufferIndex].CpuHandle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12FrameBuffer::GetDepthStencilViewCpu() const
	{
		return pDSV.CpuHandle;
	}

	INT32 D3D12FrameBuffer::GetWidth() const
	{
		return FrameSpecs.Width;
	}

	INT32 D3D12FrameBuffer::GetHeight() const
	{
		return FrameSpecs.Height;
	}
}
