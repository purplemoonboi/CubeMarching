#include "DX12RenderingApi.h"
#include "DirectX12.h"

#include "Platform/DirectX12/DX12FrameBuffer.h"


#include "Framework/Core/Log/Log.h"

namespace Engine
{
	DX12RenderingApi::~DX12RenderingApi()
	{
		if(GraphicsContext.get() != nullptr)
		{
			GraphicsContext.reset();
			GraphicsContext = nullptr;
		}

		if(FrameBuffer.get() != nullptr)
		{
			FrameBuffer.reset();
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
		/** builds our cbv descriptor etc */
		GraphicsContext = std::static_pointer_cast<DX12GraphicsContext>(GraphicsContext::Create(windowHandle, viewportWidth, viewportHeight));

		FrameBufferSpecifications fbs;
		fbs.Width = viewportWidth;
		fbs.Height = viewportHeight;

		FrameBuffer = std::static_pointer_cast<DX12FrameBuffer>(FrameBuffer::Create(fbs));
		FrameBuffer->Invalidate(GraphicsContext);

		THROW_ON_FAILURE(GraphicsContext->GraphicsCmdList->Reset(GraphicsContext->CmdListAlloc.Get(), nullptr));


		HRESULT H = GraphicsContext->GraphicsCmdList->Close();
		THROW_ON_FAILURE(H);

		ID3D12CommandList* cmdList[] = { GraphicsContext->GraphicsCmdList.Get() };
		GraphicsContext->CommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

		GraphicsContext->FlushCommandQueue();

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

	void DX12RenderingApi::SetClearColour(const float colour[4])
	{
		// Reset the command allocator
		GraphicsContext->CmdListAlloc->Reset();

		// Reset the command list
		HRESULT cmdReset = GraphicsContext->GraphicsCmdList->Reset
		(
			GraphicsContext->CmdListAlloc.Get(),
			GraphicsContext->CurrentPso
		);

		THROW_ON_FAILURE(cmdReset);

		// Reset the viewport and scissor rect whenever the command list is empty.
		GraphicsContext->GraphicsCmdList->RSSetViewports(1, &FrameBuffer->GetViewport());
		GraphicsContext->GraphicsCmdList->RSSetScissorRects(1, &FrameBuffer->GetScissorsRect());

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


		//Flush the back buffer 
		GraphicsContext->GraphicsCmdList->ClearRenderTargetView
		(
			GraphicsContext->CurrentBackBufferView(),
			DirectX::Colors::Aquamarine,
			0,
			nullptr
		);

		// Flush the depth buffer
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

		// Set the descriptors for the memory and the root signature
		ID3D12DescriptorHeap* descriptorHeaps[] = { GraphicsContext->CbvHeap.Get() };
		GraphicsContext->GraphicsCmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		GraphicsContext->GraphicsCmdList->SetGraphicsRootSignature(GraphicsContext->RootSignature.Get());

	}


	void DX12RenderingApi::DrawIndexed(const RefPointer<VertexArray>& vertexArray, INT32 indexCount)
	{
	}

	void DX12RenderingApi::DrawIndexed(const RefPointer<Geometry>& geometry, INT32 indexCount)
	{
		const auto& vertexBuffer = static_cast<DX12VertexBuffer*>(geometry->VertexBuffer.get());
		const auto& indexBuffer = static_cast<DX12IndexBuffer*>(geometry->IndexBuffer.get());



		GraphicsContext->GraphicsCmdList->IASetVertexBuffers(0, 1, &vertexBuffer->GetVertexBufferView());
		GraphicsContext->GraphicsCmdList->IASetIndexBuffer(&indexBuffer->GetIndexBufferView());
		GraphicsContext->GraphicsCmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		GraphicsContext->GraphicsCmdList->SetGraphicsRootDescriptorTable(0, GraphicsContext->CbvHeap->GetGPUDescriptorHandleForHeapStart());


		GraphicsContext->GraphicsCmdList->DrawIndexedInstanced
		(
			geometry->DrawArgs["box"].IndexCount,
			1,
			0,
			0,
			0
		);


	}

	void DX12RenderingApi::Flush()
	{
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


		THROW_ON_FAILURE(GraphicsContext->SwapChain->Present(0, 0));
		GraphicsContext->UpdateBackBufferIndex((GraphicsContext->GetBackBufferIndex() + 1) % SWAP_CHAIN_BUFFER_COUNT);


		GraphicsContext->FlushCommandQueue();
	}

}
