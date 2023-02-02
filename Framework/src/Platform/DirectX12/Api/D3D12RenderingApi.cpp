#include "D3D12RenderingApi.h"

#include "imgui.h"
#include "backends/imgui_impl_dx12.h"
#include "Platform/Directx12/Buffers/D3D12FrameBuffer.h"
#include "Platform/Directx12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/RenderItems/D3D12RenderItem.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"

#include "Framework/Core/Log/Log.h"
#include "Framework/ImGui/Platform/ImGuiImplD3D12.h"

#define NSIGHT_REPORT_FORCE_FAILED_QUERY_INTERFACES 

namespace Engine
{

//#define ENGINE_IMGUI_SUPPORT 1

	D3D12RenderingApi::~D3D12RenderingApi()
	{
		/*if(D3D12Context.get() != nullptr)
		{
			D3D12Context.reset();
			D3D12Context = nullptr;
		}

		if(D3D12FrameBuffer.get() != nullptr)
		{
			D3D12FrameBuffer.reset();
			D3D12FrameBuffer = nullptr;
		}*/
	}

	void D3D12RenderingApi::Init()
	{
		D3D12Context = nullptr;
		D3D12FrameBuffer = nullptr;
	}

	void D3D12RenderingApi::InitD3D12(HWND windowHandle, INT32 viewportWidth, INT32 viewportHeight)
	{
		/** builds our cbv descriptor etc */
		D3D12Context = std::make_unique<class D3D12Context>(windowHandle, viewportWidth, viewportHeight);

		FrameBufferSpecifications fbs;
		fbs.Width = viewportWidth;
		fbs.Height = viewportHeight;

		D3D12FrameBuffer = std::make_unique<class D3D12FrameBuffer>(fbs);
		D3D12FrameBuffer->Init(D3D12Context.get());
		D3D12FrameBuffer->ResizeFrameBuffer(D3D12Context.get());
	}

	void D3D12RenderingApi::SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		if(D3D12Context != nullptr && D3D12FrameBuffer != nullptr)
		{
			CORE_TRACE("Buffer resize");
			D3D12FrameBuffer->SetViewportDimensions(width, height);
			D3D12FrameBuffer->ResizeFrameBuffer(D3D12Context.get());
		}
	}

	void D3D12RenderingApi::ResetCommandList()
	{
		// Reset the command allocator
		// Reset the command list
		THROW_ON_FAILURE(D3D12Context->GraphicsCmdList->Reset
		(
			D3D12Context->CmdListAlloc.Get(),
			nullptr
		));
	}


	void D3D12RenderingApi::ExecCommandList()
	{
		// Execute the initialization commands.
		HRESULT closureResult = D3D12Context->GraphicsCmdList->Close();
		THROW_ON_FAILURE(closureResult);
		ID3D12CommandList* cmdsLists[] = { D3D12Context->GraphicsCmdList.Get() };
		D3D12Context->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		D3D12Context->FlushCommandQueue();
	}

	void D3D12RenderingApi::UpdateFrameResource(FrameResource* const frameResource)
	{
		CurrentFrameResource = dynamic_cast<D3D12FrameResource*>(frameResource);
		// Has the GPU finished processing the commands of the current frame resource?
		// If not, wait until the GPU has completed commands up to this fence point.
		const UINT64 a = CurrentFrameResource->Fence;
		const UINT64 b = D3D12Context->Fence->GetCompletedValue();
		if (a != 0 && b < a)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			THROW_ON_FAILURE(D3D12Context->Fence->SetEventOnCompletion(CurrentFrameResource->Fence, eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	void D3D12RenderingApi::PreRender()
	{
		CORE_ASSERT(D3D12Context->Device, "Device lost");
		CORE_ASSERT(D3D12Context->GraphicsCmdList, "Graphics CmdL lost");
		CORE_ASSERT(D3D12Context->CommandQueue, "Command queue lost");
		CORE_ASSERT(CurrentFrameResource, "No valid frame resource!");

#ifdef ENGINE_IMGUI_SUPPORT
		ImGui::Render();
#endif
		/*
		 * Reset current frame resource command allocator
		 */
		THROW_ON_FAILURE(CurrentFrameResource->CmdListAlloc->Reset());

	
		THROW_ON_FAILURE(D3D12Context->GraphicsCmdList->Reset(CurrentFrameResource->CmdListAlloc.Get(), nullptr));

	}

	void D3D12RenderingApi::PostRender()
	{
		//TODO: not sure what this can be used for just now? Already have flush which handles cmd execution.
	}

	void D3D12RenderingApi::SetClearColour(const float colour[4], PipelineStateObject* pso)
	{
		
		/**
		 * begin recording commands for rendering the opaque layer.
		 */

// Reset the viewport and scissor rect whenever the command list is empty.
		const auto dx12Pso = dynamic_cast<D3D12PipelineStateObject*>(pso);
		D3D12Context->GraphicsCmdList->SetPipelineState(dx12Pso->GetPipelineState());
		D3D12Context->GraphicsCmdList->RSSetViewports(1, &D3D12FrameBuffer->GetViewport());
		D3D12Context->GraphicsCmdList->RSSetScissorRects(1, &D3D12FrameBuffer->GetScissorsRect());

		// Indicate there will be a transition made to the resource.
		D3D12Context->GraphicsCmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				D3D12FrameBuffer->CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			)
		);

		/**
		 * Add an instruction to clear render target
		 */
		D3D12Context->GraphicsCmdList->ClearRenderTargetView
		(
			D3D12FrameBuffer->GetCurrentBackBufferViewCpu(),
			DirectX::Colors::Aquamarine,
			0,
			nullptr
		);

		/**
		 * Add an instruction to clear the depth buffer
		 */
		D3D12Context->GraphicsCmdList->ClearDepthStencilView
		(
			D3D12FrameBuffer->GetDepthStencilViewCpu(),
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			1.0f,
			0,
			0,
			nullptr
		);

		/**
		 * Set the current render target to the current back buffer
		 */
		D3D12Context->GraphicsCmdList->OMSetRenderTargets
		(
			1,
			&D3D12FrameBuffer->GetCurrentBackBufferViewCpu(),
			true,
			&D3D12FrameBuffer->GetDepthStencilViewCpu()
		);
		
	}



	void D3D12RenderingApi::DrawOpaqueItems
	(
		const std::vector<RenderItem*>& renderItems,
		UINT currentFrameResourceIndex
	)
	{
		/**
		 * Set the descriptor heaps for the constant buffers (inc pass buffer)
		 * Bind the root signature
		 */
		ID3D12DescriptorHeap* descriptorHeaps[] = { D3D12Context->CbvHeap.Get() };
		D3D12Context->GraphicsCmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		D3D12Context->GraphicsCmdList->SetGraphicsRootSignature(D3D12Context->RootSignature.Get());

		/**
		*	The pass buffer is *bound* to the 1st register.
		*/
		const D3D12_GPU_VIRTUAL_ADDRESS passBufferAddress = CurrentFrameResource->PassBuffer->Resource()->GetGPUVirtualAddress();
		D3D12Context->GraphicsCmdList->SetGraphicsRootConstantBufferView(2, passBufferAddress);

		const auto opaqueItemCount = static_cast<UINT>(renderItems.size());

		// For each render item...
		for (auto& renderItem : renderItems)
		{
			const auto renderItemDerived = dynamic_cast<D3D12RenderItem*>(renderItem);
			const auto d3d12VertexBuffer  = dynamic_cast<D3D12VertexBuffer*>(renderItem->Geometry->VertexBuffer.get());
			const auto d3d12IndexBuffer   = dynamic_cast<D3D12IndexBuffer*>(renderItem->Geometry->IndexBuffer.get());

			/**
			 * bind the vertex and index buffers
			 */
			D3D12Context->GraphicsCmdList->IASetVertexBuffers(0, 1, &d3d12VertexBuffer->GetVertexBufferView());
			D3D12Context->GraphicsCmdList->IASetIndexBuffer(&d3d12IndexBuffer->GetIndexBufferView());
			D3D12Context->GraphicsCmdList->IASetPrimitiveTopology(renderItemDerived->PrimitiveType);

			/**
			 *	The constant buffer is *bound* to the 0th register.
			 */
			const UINT objConstBufferByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));
			const UINT matConstBufferByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(MaterialConstants));

			ID3D12Resource* objectConstantBuffer	 = CurrentFrameResource->ConstantBuffer->Resource();
			ID3D12Resource* materialConstantBuffer = CurrentFrameResource->MaterialBuffer->Resource();

			const D3D12_GPU_VIRTUAL_ADDRESS objConstBufferAddress = objectConstantBuffer->GetGPUVirtualAddress() + renderItem->ObjectConstantBufferIndex * objConstBufferByteSize;
			const D3D12_GPU_VIRTUAL_ADDRESS materialBufferAddress = materialConstantBuffer->GetGPUVirtualAddress() + renderItem->Material->GetBufferIndex() * matConstBufferByteSize;

			D3D12Context->GraphicsCmdList->SetGraphicsRootConstantBufferView(0, objConstBufferAddress);
			D3D12Context->GraphicsCmdList->SetGraphicsRootConstantBufferView(1, materialBufferAddress);

			D3D12Context->GraphicsCmdList->DrawIndexedInstanced
			(
				renderItem->IndexCount,
				1,
				renderItem->StartIndexLocation, 
				renderItem->BaseVertexLocation, 
				0
			);
		}

	}

	void D3D12RenderingApi::Flush()
	{
#ifdef ENGINE_IMGUI_SUPPORT
		D3D12Context->GraphicsCmdList->SetDescriptorHeaps(1, ImGuiImplD3D12::ImGuiHeap.GetAddressOf());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), D3D12Context->GraphicsCmdList.Get());
#endif

		// Now instruct we have made the changes to the buffer
		D3D12Context->GraphicsCmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				D3D12FrameBuffer->CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT
			)
		);


		THROW_ON_FAILURE(D3D12Context->GraphicsCmdList->Close());
		ID3D12CommandList* cmdsLists[] = { D3D12Context->GraphicsCmdList.Get() };
		D3D12Context->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

#ifdef ENGINE_IMGUI_SUPPORT
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, (void*)D3D12Context->GraphicsCmdList.Get());
		}
#endif
	
		THROW_ON_FAILURE(D3D12Context->SwapChain->Present(0, 0));
		D3D12FrameBuffer->SetBackBufferIndex((D3D12FrameBuffer->GetBackBufferIndex() + 1) % SWAP_CHAIN_BUFFER_COUNT);
		CurrentFrameResource->Fence = ++D3D12Context->GPU_TO_CPU_SYNC_COUNT;
		/**
		 * Add an instruction to the command queue to set a new fence point. 
		 * Because we are on the GPU timeline, the new fence point won't be 
		 * set until the GPU finishes processing all the commands prior to this Signal().
		 */
		D3D12Context->CommandQueue->Signal(D3D12Context->Fence.Get(), D3D12Context->GPU_TO_CPU_SYNC_COUNT);
	}

}
