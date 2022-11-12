#include "Framework/cmpch.h"
#include "DX12FrameBuffer.h"
#include "DX12GraphicsContext.h"

#include "Framework/Core/Log/Log.h"

namespace Engine
{


	DX12FrameBuffer::DX12FrameBuffer(const FrameBufferSpecifications& fBufferSpecs)
		:
		ViewportWidth(fBufferSpecs.Width),
		ViewportHeight(fBufferSpecs.Height)
	{
		
	}

	void DX12FrameBuffer::Bind()
	{

	}

	void DX12FrameBuffer::UnBind()
	{

	}

	void DX12FrameBuffer::RebuildFrameBuffer(INT32 width, INT32 height)
	{
	}

	void DX12FrameBuffer::Invalidate(RefPointer<DX12GraphicsContext> graphicsContext)
	{
		CORE_ASSERT(graphicsContext->Device, "The 'D3D device' has failed...");
		CORE_ASSERT(graphicsContext->SwapChain, "The 'swap chain' has failed...");
		CORE_ASSERT(graphicsContext->GraphicsCmdList, "The 'graphics command list' has failed...");

		// Flush before changing any resources.
		graphicsContext->FlushCommandQueue();

	    THROW_ON_FAILURE(graphicsContext->GraphicsCmdList->Reset(graphicsContext->CmdListAlloc.Get(), nullptr));

		// Release the previous resources we will be recreating.
		for (UINT i = 0; i < Engine::SWAP_CHAIN_BUFFER_COUNT; ++i)
		{
			graphicsContext->SwapChainBuffer[i].Reset();
		}

		graphicsContext->DepthStencilBuffer.Reset();


		// Resize the swap chain.
		THROW_ON_FAILURE(graphicsContext->SwapChain->ResizeBuffers
		(
			SWAP_CHAIN_BUFFER_COUNT,
			ViewportWidth, ViewportHeight,
			graphicsContext->GetBackBufferFormat(),
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		));



		graphicsContext->SetBackBufferIndex(0);



		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(graphicsContext->RtvHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
		{
			graphicsContext->SwapChain->GetBuffer(i, IID_PPV_ARGS(&graphicsContext->SwapChainBuffer[i]));

			graphicsContext->Device->CreateRenderTargetView
			(
				graphicsContext->SwapChainBuffer[i].Get(),
				nullptr, 
				rtvHeapHandle
			);

			THROW_ON_FAILURE(graphicsContext->Device->GetDeviceRemovedReason());

			rtvHeapHandle.Offset(1, graphicsContext->GetRtvDescSize());
		}


		// Create the depth/stencil buffer and view.
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = ViewportWidth;
		depthStencilDesc.Height = ViewportHeight;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
		// the depth buffer.  Therefore, because we need to create two views to the same resource:
		//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
		//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
		// we need to create the depth buffer resource with a typeless format.  
		depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthStencilDesc.SampleDesc.Count = graphicsContext->GetMsaaState() ? 4 : 1;
		depthStencilDesc.SampleDesc.Quality = graphicsContext->GetMsaaState() ? (graphicsContext->GetMsaaQaulity() - 1) : 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


		D3D12_CLEAR_VALUE optClear;
		optClear.Format = graphicsContext->GetDepthStencilFormat();
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;


		THROW_ON_FAILURE(graphicsContext->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(graphicsContext->DepthStencilBuffer.GetAddressOf())
		));

		THROW_ON_FAILURE(graphicsContext->Device->GetDeviceRemovedReason());

		// Create descriptor to mip level 0 of entire resource using the format of the resource.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;

		graphicsContext->Device->CreateDepthStencilView
		(
			graphicsContext->DepthStencilBuffer.Get(), 
			&dsvDesc, 
			graphicsContext->DepthStencilView()
		);


		// Transition the resource from its initial state to be used as a depth buffer.
		graphicsContext->GraphicsCmdList->ResourceBarrier
		(
			1, 
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				graphicsContext->DepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_DEPTH_WRITE
			)
		);

		// Execute the resize commands.
		THROW_ON_FAILURE(graphicsContext->GraphicsCmdList->Close());

		ID3D12CommandList* cmdsLists[] = { graphicsContext->GraphicsCmdList.Get() };

		graphicsContext->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// Wait until resize is complete.
		graphicsContext->FlushCommandQueue();


		// Update the viewport transform to cover the client area.
		ScreenViewport.TopLeftX = 0;
		ScreenViewport.TopLeftY = 0;
		ScreenViewport.Width = static_cast<float>(ViewportWidth);
		ScreenViewport.Height = static_cast<float>(ViewportHeight);
		ScreenViewport.MinDepth = 0.0f;
		ScreenViewport.MaxDepth = 1.0f;

		ScissorRect = { 0, 0, ViewportWidth, ViewportHeight };


	}
}
