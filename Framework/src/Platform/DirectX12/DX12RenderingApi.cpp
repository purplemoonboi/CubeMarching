#include "DX12RenderingApi.h"
#include "DirectX12.h"
#include "DX12Shader.h"

#include "Platform/DirectX12/DX12FrameBuffer.h"


#include "Framework/Core/Log/Log.h"

namespace Engine
{
	DX12RenderingApi::~DX12RenderingApi()
	{
		/*if(GraphicsContext.get() != nullptr)
		{
			GraphicsContext.reset();
			GraphicsContext = nullptr;
		}

		if(FrameBuffer.get() != nullptr)
		{
			FrameBuffer.reset();
			FrameBuffer = nullptr;
		}*/
	}

	void DX12RenderingApi::Init()
	{
		GraphicsContext = nullptr;
		FrameBuffer = nullptr;
	}

	void DX12RenderingApi::InitD3D(HWND windowHandle, INT32 viewportWidth, INT32 viewportHeight)
	{
		/** builds our cbv descriptor etc */
		GraphicsContext = std::dynamic_pointer_cast<DX12GraphicsContext>(GraphicsContext::Create(windowHandle, viewportWidth, viewportHeight));

		FrameBufferSpecifications fbs;
		fbs.Width = viewportWidth;
		fbs.Height = viewportHeight;

		FrameBuffer = std::static_pointer_cast<DX12FrameBuffer>(FrameBuffer::Create(fbs));
		FrameBuffer->ResizeFrameBuffer(GraphicsContext);

	}

	void DX12RenderingApi::SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		if(GraphicsContext != nullptr && FrameBuffer != nullptr)
		{
			CORE_TRACE("Buffer resize");
			//FrameBuffer->SetViewportDimensions(width, height);
			//FrameBuffer->ResizeFrameBuffer(GraphicsContext);
		}
	}

	void DX12RenderingApi::SetClearColour(const float colour[4])
	{

		CORE_ASSERT(GraphicsContext->Device, "Device lost");
		CORE_ASSERT(GraphicsContext->GraphicsCmdList, "Graphics CmdL lost");
		CORE_ASSERT(GraphicsContext->CommandQueue, "Command queue lost");

		// Reset the command allocator
		GraphicsContext->CmdListAlloc->Reset();

		// Reset the command list
		HRESULT cmdReset = GraphicsContext->GraphicsCmdList->Reset
		(
			GraphicsContext->CmdListAlloc.Get(),
			GraphicsContext->CurrentPso.Get()
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


		//ExecCommandList the back buffer 
		GraphicsContext->GraphicsCmdList->ClearRenderTargetView
		(
			GraphicsContext->CurrentBackBufferView(),
			DirectX::Colors::Aquamarine,
			0,
			nullptr
		);

		// ExecCommandList the depth buffer
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

	void DX12RenderingApi::Flush()
	{
	}

	void DX12RenderingApi::ResetCommandList()
	{

		CORE_ASSERT(GraphicsContext->Device, "Device lost");
		CORE_ASSERT(GraphicsContext->GraphicsCmdList, "Graphics CmdL lost");
		CORE_ASSERT(GraphicsContext->CommandQueue, "Command queue lost");

		// Reset the command allocator
		// Reset the command list
		HRESULT cmdReset = GraphicsContext->GraphicsCmdList->Reset
		(
			GraphicsContext->CmdListAlloc.Get(),
			nullptr
		);

		THROW_ON_FAILURE(cmdReset);

	}

	void DX12RenderingApi::DrawIndexed(const RefPointer<Geometry>& geometry, INT32 indexCount, PipelineStateObject* pso)
	{

		const auto dx12Pso = dynamic_cast<DX12PipelineStateObject*>(pso);


		CORE_ASSERT(GraphicsContext->Device, "Device lost");
		CORE_ASSERT(GraphicsContext->GraphicsCmdList, "Graphics CmdL lost");
		CORE_ASSERT(GraphicsContext->CommandQueue, "Command queue lost");

		// Reset the command allocator
		THROW_ON_FAILURE(GraphicsContext->CmdListAlloc->Reset());


		// Reset the command list
		HRESULT cmdReset = GraphicsContext->GraphicsCmdList->Reset
		(
			GraphicsContext->CmdListAlloc.Get(),
			dx12Pso->GetPipelineState()
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


		//ExecCommandList the back buffer 
		GraphicsContext->GraphicsCmdList->ClearRenderTargetView
		(
			GraphicsContext->CurrentBackBufferView(),
			DirectX::Colors::Cyan,
			0,
			nullptr
		);

		// ExecCommandList the depth buffer
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

		HRESULT cb = GraphicsContext->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(cb);

		GraphicsContext->GraphicsCmdList->SetGraphicsRootSignature(GraphicsContext->RootSignature.Get());


		const auto& vertexBuffer = dynamic_cast<DX12VertexBuffer*>(geometry->VertexBuffer.get());
		const auto& indexBuffer  = dynamic_cast<DX12IndexBuffer*>(geometry->IndexBuffer.get());


		GraphicsContext->GraphicsCmdList->IASetVertexBuffers(0, 1, &vertexBuffer->GetVertexBufferView());
		GraphicsContext->GraphicsCmdList->IASetIndexBuffer(&indexBuffer->GetIndexBufferView());
		GraphicsContext->GraphicsCmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		GraphicsContext->GraphicsCmdList->SetGraphicsRootDescriptorTable(0, GraphicsContext->CbvHeap->GetGPUDescriptorHandleForHeapStart());


		GraphicsContext->GraphicsCmdList->DrawIndexedInstanced
		(
			geometry->DrawArgs["Box"].IndexCount,
			1,
			0,
			0,
			0
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
		HRESULT closedResult = GraphicsContext->GraphicsCmdList->Close();
		THROW_ON_FAILURE(closedResult);


		ID3D12CommandList* cmdsLists[] = { GraphicsContext->GraphicsCmdList.Get() };
		GraphicsContext->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		HRESULT swapResult = GraphicsContext->SwapChain->Present(0, 0);
		THROW_ON_FAILURE(swapResult);

		auto index = GraphicsContext->GetBackBufferIndex();
		GraphicsContext->UpdateBackBufferIndex((index + 1) % SWAP_CHAIN_BUFFER_COUNT);


		GraphicsContext->FlushCommandQueue();

	}




	void DX12RenderingApi::ExecCommandList()
	{
		CORE_ASSERT(GraphicsContext->Device, "Device lost");
		CORE_ASSERT(GraphicsContext->GraphicsCmdList, "Graphics CmdL lost");
		CORE_ASSERT(GraphicsContext->CommandQueue, "Command queue lost");

		// Execute the initialization commands.
		HRESULT result = S_OK;
		result = GraphicsContext->GraphicsCmdList->Close();
		THROW_ON_FAILURE(result);

		ID3D12CommandList* cmdsLists[] = { GraphicsContext->GraphicsCmdList.Get() };
		GraphicsContext->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		GraphicsContext->FlushCommandQueue();
	}

}
