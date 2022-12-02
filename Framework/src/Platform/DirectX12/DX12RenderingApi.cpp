#include "DirectX12.h"

#include "DX12Buffer.h"
#include "DX12RenderingApi.h"
#include "DX12Shader.h"
#include "DX12RenderItem.h"
#include "DX12PipelineStateObject.h"
#include "DX12FrameResource.h"
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
		GraphicsContext = std::static_pointer_cast<DX12GraphicsContext>(GraphicsContext::Create(windowHandle, viewportWidth, viewportHeight));

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


	void DX12RenderingApi::ResetCommandList()
	{
		// Reset the command allocator
		// Reset the command list
		THROW_ON_FAILURE(GraphicsContext->GraphicsCmdList->Reset
		(
			GraphicsContext->CmdListAlloc.Get(),
			nullptr
		));

	}


	void DX12RenderingApi::ExecCommandList()
	{
		// Execute the initialization commands.
		THROW_ON_FAILURE(GraphicsContext->GraphicsCmdList->Close());
		ID3D12CommandList* cmdsLists[] = { GraphicsContext->GraphicsCmdList.Get() };
		GraphicsContext->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		GraphicsContext->FlushCommandQueue();
	}

	void DX12RenderingApi::UpdateFrameResource(FrameResource* const frameResource)
	{
		CurrentFrameResource = dynamic_cast<DX12FrameResource*>(frameResource);
		// Has the GPU finished processing the commands of the current frame resource?
		// If not, wait until the GPU has completed commands up to this fence point.
		auto a = CurrentFrameResource->Fence;
		auto b = GraphicsContext->Fence->GetCompletedValue();
		if (a != 0 && b < a)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			THROW_ON_FAILURE(GraphicsContext->Fence->SetEventOnCompletion(CurrentFrameResource->Fence, eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	void DX12RenderingApi::SetClearColour(const float colour[4], PipelineStateObject* pso)
	{
		CORE_ASSERT(GraphicsContext->Device, "Device lost");
		CORE_ASSERT(GraphicsContext->GraphicsCmdList, "Graphics CmdL lost");
		CORE_ASSERT(GraphicsContext->CommandQueue, "Command queue lost");
		CORE_ASSERT(CurrentFrameResource, "No valid frame resource!");


		/*
		 * Reset current frame resource command allocator
		 */
		auto cmdListAlloc = CurrentFrameResource->CmdListAlloc;
		THROW_ON_FAILURE(cmdListAlloc->Reset());



		const auto dx12Pso = dynamic_cast<DX12PipelineStateObject*>(pso);
		HRESULT cmdReset = GraphicsContext->GraphicsCmdList->Reset(cmdListAlloc.Get(), dx12Pso->GetPipelineState());
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





		/**
		 * Add an instruction to clear render target
		 */
		GraphicsContext->GraphicsCmdList->ClearRenderTargetView
		(
			GraphicsContext->CurrentBackBufferView(),
			DirectX::Colors::Aquamarine,
			0,
			nullptr
		);



		/**
		 * Add an instruction to clear the depth buffer
		 */
		GraphicsContext->GraphicsCmdList->ClearDepthStencilView
		(
			GraphicsContext->DepthStencilView(),
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			1.0f,
			0,
			0,
			nullptr
		);




		/**
		 * Set the current render target to the current back buffer
		 */
		GraphicsContext->GraphicsCmdList->OMSetRenderTargets
		(
			1,
			&GraphicsContext->CurrentBackBufferView(),
			true,
			&GraphicsContext->DepthStencilView()
		);


		/**
		 *
		 * Set the descriptor heaps for the constant buffers (inc pass buffer)
		 *
		 * Bind the root signature
		 *
		 */
		ID3D12DescriptorHeap* descriptorHeaps[] = { GraphicsContext->CbvHeap.Get() };
		GraphicsContext->GraphicsCmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		GraphicsContext->GraphicsCmdList->SetGraphicsRootSignature(GraphicsContext->RootSignature.Get());



	}


	void DX12RenderingApi::DrawOpaqueItems
	(
		const std::vector<RenderItem*>& renderItems,
		UINT currentFrameResourceIndex
	)
	{
		/**
		*	The pass buffer is *bound* to the 1st register.
		*/
		const D3D12_GPU_VIRTUAL_ADDRESS passBufferAddress = CurrentFrameResource->PassBuffer->Resource()->GetGPUVirtualAddress();
		GraphicsContext->GraphicsCmdList->SetGraphicsRootConstantBufferView(2, passBufferAddress);

		const auto opaqueItemCount = static_cast<UINT>(renderItems.size());

		// For each render item...
		for (auto& renderItem : renderItems)
		{
			const auto renderItemDerived = dynamic_cast<DX12RenderItem*>(renderItem);
			const auto dx12VertexBuffer  = dynamic_cast<DX12VertexBuffer*>(renderItem->Geometry->VertexBuffer.get());
			const auto dx12IndexBuffer   = dynamic_cast<DX12IndexBuffer*>(renderItem->Geometry->IndexBuffer.get());

			/**
			 * bind the vertex and index buffers
			 */
			GraphicsContext->GraphicsCmdList->IASetVertexBuffers(0, 1, &dx12VertexBuffer->GetVertexBufferView());
			GraphicsContext->GraphicsCmdList->IASetIndexBuffer(&dx12IndexBuffer->GetIndexBufferView());
			GraphicsContext->GraphicsCmdList->IASetPrimitiveTopology(renderItemDerived->PrimitiveType);

			/**
			 *	The constant buffer is *bound* to the 0th register.
			 */
			const UINT objConstBufferByteSize = DX12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));
			const UINT matConstBufferByteSize = DX12BufferUtils::CalculateConstantBufferByteSize(sizeof(MaterialConstants));

			ID3D12Resource* objectConstantBuffer	 = CurrentFrameResource->ConstantBuffer->Resource();
			ID3D12Resource* materialConstantBuffer = CurrentFrameResource->MaterialBuffer->Resource();

			const D3D12_GPU_VIRTUAL_ADDRESS objConstBufferAddress = objectConstantBuffer->GetGPUVirtualAddress() + (renderItem->ObjectConstantBufferIndex * objConstBufferByteSize);
			const D3D12_GPU_VIRTUAL_ADDRESS materialBufferAddress = materialConstantBuffer->GetGPUVirtualAddress() + (renderItem->Material->GetBufferIndex() * matConstBufferByteSize);

			GraphicsContext->GraphicsCmdList->SetGraphicsRootConstantBufferView(0, objConstBufferAddress);
			GraphicsContext->GraphicsCmdList->SetGraphicsRootConstantBufferView(1, materialBufferAddress);

			/**
			 *
			 */
			GraphicsContext->GraphicsCmdList->DrawIndexedInstanced
			(
				renderItem->IndexCount,
				1,
				renderItem->StartIndexLocation, 
				renderItem->BaseVertexLocation, 
				0
			);
		}

		

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

		/**
		 * Close graphics command list.
		 */
		THROW_ON_FAILURE(GraphicsContext->GraphicsCmdList->Close());

		/**
		 * Execute the current recorded commands on the GPU
		 */
		GraphicsContext->ExecuteGraphicsCommandList();

		/**
		 * Swap back buffers
		 */
		GraphicsContext->SwapBuffers();

		/**
		 * Advance the fence value to mark commands up to this fence point.
		 */ 
		CurrentFrameResource->Fence = ++GraphicsContext->GPU_TO_CPU_SYNC_COUNT;
		/**
		 * Add an instruction to the command queue to set a new fence point. 
		 * Because we are on the GPU timeline, the new fence point won't be 
		 * set until the GPU finishes processing all the commands prior to this Signal().
		 */
		GraphicsContext->SignalGPU();
	}











	void DX12RenderingApi::DrawIndexed(const ScopePointer<MeshGeometry>& geometry, INT32 indexCount)
	{


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


		auto vertexBuffer = dynamic_cast<DX12VertexBuffer*>(geometry->VertexBuffer.get());
		auto indexBuffer = dynamic_cast<DX12IndexBuffer*>(geometry->IndexBuffer.get());


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

	}

}
