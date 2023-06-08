#include "Framework/cmpch.h"
#include "D3D12FrameBuffer.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Utilities/D3D12BufferFactory.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Core/D3D12Core.h"

namespace Foundation::Graphics::D3D12
{
	D3D12FrameBuffer::D3D12FrameBuffer(const FrameBufferSpecifications& fBufferSpecs)
		:
		FrameBufferSpecs(fBufferSpecs),
		BackBufferIndex(0)
	{
		ScissorRect = {};
		ScreenViewport = {};
	}

	D3D12FrameBuffer::~D3D12FrameBuffer()
	{}

	void D3D12FrameBuffer::Init(GraphicsContext* context, FrameBufferSpecifications& fbs)
	{
		Context = dynamic_cast<D3D12Context*>(context);
		OnResizeFrameBuffer(fbs);
	}

	void D3D12FrameBuffer::Bind(void* args)
	{
		auto* commandList = static_cast<ID3D12GraphicsCommandList*>(args);

		CORE_ASSERT(commandList, "The 'graphics command list' has failed...");

		// Indicate there will be a transition made to the resource.
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			SwapChainBuffer[BackBufferIndex].Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));

	
		commandList->RSSetViewports(1, &ScreenViewport);
		commandList->RSSetScissorRects(1, &ScissorRect);

		commandList->OMSetRenderTargets(1,
			&RenderTargetHandles[BackBufferIndex],
			true, &DepthStencilHandle
		);

	}

	void D3D12FrameBuffer::UnBind(void* args)
	{
		auto* commandList = static_cast<ID3D12GraphicsCommandList*>(args);
		CORE_ASSERT(commandList, "The 'graphics command list' has failed...");

		commandList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				SwapChainBuffer[BackBufferIndex].Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT
			)
		);
	}

	void D3D12FrameBuffer::OnResizeFrameBuffer(FrameBufferSpecifications& specifications)
	{
		HRESULT hr{ S_OK };
		CORE_ASSERT(pDevice, "The 'D3D device' has failed...");
		CORE_ASSERT(Context->pSwapChain, "The 'swap chain' has failed...");
		CORE_ASSERT(Context->pGCL, "The 'graphics command list' has failed...");

		FrameBufferSpecs = specifications;

		// Flush before changing any resources.
		Context->FlushCommandQueue();

		hr = Context->pGCL->Reset(Context->pCmdAlloc.Get(), nullptr);
		THROW_ON_FAILURE(hr);

		// Release the previous resources we will be recreating.
		for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		{
			SwapChainBuffer[i].Reset();
		}

		DepthStencilBuffer.Reset();


		// Resize the swap chain.
		hr = Context->pSwapChain->ResizeBuffers
		(
			SWAP_CHAIN_BUFFER_COUNT,
			FrameBufferSpecs.Width, FrameBufferSpecs.Height,
			BackBufferFormat,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		);
		THROW_ON_FAILURE(hr);


		BackBufferIndex = 0;

		//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(Context->RtvHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_RENDER_TARGET_VIEW_DESC desc = {};
		

		for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
		{
			Context->pSwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffer[i]));

			pDevice->CreateRenderTargetView
			(
				SwapChainBuffer[i].Get(),
				nullptr,
				RenderTargetHandles[i]
			);
			SwapChainBuffer->Get()->SetName(L"Swap Chain");
			THROW_ON_FAILURE(pDevice->GetDeviceRemovedReason());
			//rtvHeapHandle.Offset(1, RtvDescriptorSize);
		}

		// Create the depth/stencil buffer and view.
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = FrameBufferSpecs.Width;
		depthStencilDesc.Height = FrameBufferSpecs.Height;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;

		// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
		// the depth buffer.  Therefore, because we need to create two views to the same resource:
		//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
		//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
		// we need to create the depth buffer resource with a typeless format.  
		depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthStencilDesc.SampleDesc.Count = Context->GetMsaaState() ? 4 : 1;
		depthStencilDesc.SampleDesc.Quality = Context->GetMsaaState() ? (Context->GetMsaaQaulity() - 1) : 0;
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
		Context->pGCL->ResourceBarrier
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
		hr = Context->pGCL->Close();
		THROW_ON_FAILURE(hr);

		ID3D12CommandList* cmdsLists[] = { Context->pGCL.Get() };
		Context->pQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// Wait until resize is complete.
		Context->FlushCommandQueue();

		// Update the viewport transform to cover the client area.
		ScreenViewport.TopLeftX = 0.0f;
		ScreenViewport.TopLeftY = 0.0f;
		ScreenViewport.Width = static_cast<float>(FrameBufferSpecs.Width);
		ScreenViewport.Height = static_cast<float>(FrameBufferSpecs.Height);
		ScreenViewport.MinDepth = 0.0f;
		ScreenViewport.MaxDepth = 1.0f;

		ScissorRect = { 0, 0, FrameBufferSpecs.Width, FrameBufferSpecs.Height };
	}

	

	void D3D12FrameBuffer::SetBufferSpecifications(FrameBufferSpecifications& fbSpecs)
	{
		FrameBufferSpecs.Width = fbSpecs.Width;
		FrameBufferSpecs.Height = fbSpecs.Height;
		FrameBufferSpecs.OffsetX = fbSpecs.OffsetX;
		FrameBufferSpecs.OffsetY = fbSpecs.OffsetY;
	}

	UINT64 D3D12FrameBuffer::GetFrameBuffer() const
	{
		return 0;
	}

	ID3D12Resource* D3D12FrameBuffer::CurrentBackBuffer() const
	{
		return SwapChainBuffer[BackBufferIndex].Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12FrameBuffer::GetCurrentBackBufferViewCpu() const
	{
		return RenderTargetHandles[BackBufferIndex];
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12FrameBuffer::GetDepthStencilViewCpu() const
	{
		return DepthStencilHandle;
	}

	INT32 D3D12FrameBuffer::GetWidth() const
	{
		return FrameBufferSpecs.Width;
	}

	INT32 D3D12FrameBuffer::GetHeight() const
	{
		return FrameBufferSpecs.Height;
	}
}
