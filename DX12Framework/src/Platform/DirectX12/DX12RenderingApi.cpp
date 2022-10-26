#include "DX12RenderingApi.h"
#include "DirectX12.h"

#include "Platform/DirectX12/DX12GraphicsContext.h"
#include "Platform/DirectX12/DX12FrameBuffer.h"


#include "Framework/Core/Log/Log.h"

namespace DX12Framework
{
	DX12RenderingApi::~DX12RenderingApi()
	{
		if(GraphicsContext)
		{
			delete GraphicsContext;
			GraphicsContext = nullptr;
		}

		if(FrameBuffer)
		{
			delete FrameBuffer;
			FrameBuffer = nullptr;
		}
	}

	void DX12RenderingApi::Init()
	{
		GraphicsContext = nullptr;
		FrameBuffer = nullptr;
	}

	void DX12RenderingApi::InitD3D(HWND windowHandle, INT32 viewportWidth, INT32 viewportHeight)
	{
		GraphicsContext = new DX12GraphicsContext(windowHandle, viewportWidth, viewportHeight);

		FrameBufferSpecifications fbs;
		fbs.Width = viewportWidth;
		fbs.Height = viewportHeight;

		IsResizing = true;
		FrameBuffer = new DX12FrameBuffer(fbs);
		FrameBuffer->Invalidate(GraphicsContext);
		IsResizing = false;
	}

	void DX12RenderingApi::SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		if(GraphicsContext != nullptr && FrameBuffer != nullptr)
		{
			CORE_TRACE("Buffer resize");
			FrameBuffer->SetViewportDimensions(width, height);
			FrameBuffer->Invalidate(GraphicsContext);
		}
	}

	void DX12RenderingApi::SetClearColour(DirectX::XMFLOAT4 colour)
	{

	}

	void DX12RenderingApi::Draw()
	{
		if (GraphicsContext != nullptr && !IsResizing)
		{
			// Reset the command allocator
			GraphicsContext->DirCmdListAlloc->Reset();




			// Reset the command list
			GraphicsContext->GraphicsCmdList->Reset
			(
				GraphicsContext->DirCmdListAlloc.Get(),
				nullptr
			);





			// Indicate there will be a transition made to the resource.
			GraphicsContext->GraphicsCmdList->ResourceBarrier
			(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition
				(
					GraphicsContext->CurrentBackBuffer(),
					D3D12_RESOURCE_STATE_PRESENT,
					D3D12_RESOURCE_STATE_RENDER_TARGET
				)
			);





			// Reset the viewport and scissor rect whenever the command list is empty.
			GraphicsContext->GraphicsCmdList->RSSetViewports(1, &FrameBuffer->GetViewport());
			GraphicsContext->GraphicsCmdList->RSSetScissorRects(1, &FrameBuffer->GetScissorsRect());





			//Clear the back buffer 
			GraphicsContext->GraphicsCmdList->ClearRenderTargetView
			(
				GraphicsContext->CurrentBackBufferView(),
				DirectX::Colors::Red,
				0,
				nullptr
			);





			// Clear the depth buffer
			GraphicsContext->GraphicsCmdList->ClearDepthStencilView
			(
				GraphicsContext->DepthStencilView(),
				D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
				1.0f,
				0,
				0,
				nullptr
			);





			// Set the render targets descriptors
			GraphicsContext->GraphicsCmdList->OMSetRenderTargets
			(
				1,
				&GraphicsContext->CurrentBackBufferView(),
				true,
				&GraphicsContext->DepthStencilView()
			);





			// Now instruct we have made the changes to the buffer
			GraphicsContext->GraphicsCmdList->ResourceBarrier
			(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition
				(
					GraphicsContext->CurrentBackBuffer(),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_PRESENT
				)
			);





			// We can now close the command list
			GraphicsContext->GraphicsCmdList->Close();





			ID3D12CommandList* cmdsLists[] = { GraphicsContext->GraphicsCmdList.Get() };
			GraphicsContext->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);


			GraphicsContext->SwapChain->Present(0, 0);

			GraphicsContext->UpdateBackBufferIndex((GraphicsContext->GetBackBufferIndex() + 1) % SwapChainBufferCount);


			GraphicsContext->FlushCommandQueue();
		}
	}

	void DX12RenderingApi::Clear()
	{

	}

}