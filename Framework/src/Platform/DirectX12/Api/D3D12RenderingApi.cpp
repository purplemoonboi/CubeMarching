#include "D3D12RenderingApi.h"

#include "Platform/Directx12/Buffers/D3D12FrameBuffer.h"
#include "Platform/Directx12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/RenderItems/D3D12RenderItem.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"

#include "Framework/Core/Log/Log.h"
#include "Framework/ImGui/Platform/ImGuiImplD3D12.h"

#include <imgui.h>
#include <backends/imgui_impl_dx12.h>

namespace Engine
{

//#define ENGINE_IMGUI_SUPPORT 1

	D3D12RenderingApi::~D3D12RenderingApi()
	{
	}

	void D3D12RenderingApi::Init(GraphicsContext* context, INT32 viewportWidth, INT32 viewportHeight)
	{
		Context = dynamic_cast<D3D12Context*>(context);


		D3D12MemoryManager = std::make_unique<class D3D12MemoryManager>();
		D3D12MemoryManager->InitialiseSrvUavHeap(Context, 32);

		FrameBufferSpecifications fbs;
		fbs.Width = viewportWidth;
		fbs.Height = viewportHeight;

		FrameBuffer = std::make_unique<class D3D12FrameBuffer>(fbs);
		FrameBuffer->Init(Context);
		FrameBuffer->RebuildFrameBuffer(fbs.Width, fbs.Height);
	}

	void D3D12RenderingApi::SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		if(Context != nullptr && FrameBuffer != nullptr)
		{
			CORE_TRACE("Buffer resize");
			FrameBuffer->RebuildFrameBuffer(width, height);
		}
	}

	void D3D12RenderingApi::ResetCommandList()
	{
		// Reset the command allocator
		// Reset the command list
		THROW_ON_FAILURE(Context->GraphicsCmdList->Reset
		(
			Context->CmdListAlloc.Get(),
			nullptr
		));
	}


	void D3D12RenderingApi::ExecCommandList()
	{
		// Execute the initialization commands.
		HRESULT closureResult = Context->GraphicsCmdList->Close();
		THROW_ON_FAILURE(closureResult);
		ID3D12CommandList* cmdsLists[] = { Context->GraphicsCmdList.Get() };
		Context->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		Context->FlushCommandQueue();
	}

	void D3D12RenderingApi::UpdateFrameResource(FrameResource* const frameResource)
	{
		CurrentFrameResource = dynamic_cast<D3D12FrameResource*>(frameResource);
		// Has the GPU finished processing the commands of the current frame resource?
		// If not, wait until the GPU has completed commands up to this fence point.
		const UINT64 a = CurrentFrameResource->Fence;
		const UINT64 b = Context->Fence->GetCompletedValue();
		if (a != 0 && b < a)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			THROW_ON_FAILURE(Context->Fence->SetEventOnCompletion(CurrentFrameResource->Fence, eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	void D3D12RenderingApi::PreRender()
	{
		CORE_ASSERT(Context->Device, "Device lost");
		CORE_ASSERT(Context->GraphicsCmdList, "Graphics CmdL lost");
		CORE_ASSERT(Context->CommandQueue, "Command queue lost");
		CORE_ASSERT(CurrentFrameResource, "No valid frame resource!");

#ifdef ENGINE_IMGUI_SUPPORT
		ImGui::Render();
#endif
		const HRESULT cmdResetResult = CurrentFrameResource->CmdListAlloc->Reset();
		THROW_ON_FAILURE(cmdResetResult);
		const HRESULT cmdListResult = Context->GraphicsCmdList->Reset(CurrentFrameResource->CmdListAlloc.Get(), nullptr);
		THROW_ON_FAILURE(cmdListResult);

		Context->GraphicsCmdList->ClearRenderTargetView
		(
			FrameBuffer->GetCurrentBackBufferViewCpu(),
			DirectX::Colors::Aquamarine,
			0,
			nullptr
		);

		Context->GraphicsCmdList->ClearDepthStencilView(FrameBuffer->GetDepthStencilViewCpu(),
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr
		);

	}

	void D3D12RenderingApi::PostRender()
	{
		//TODO: not sure what this can be used for just now? Already have flush which handles cmd execution.
	}

	void D3D12RenderingApi::BindRenderPass(PipelineStateObject* pso)
	{

		const auto dx12Pso = dynamic_cast<D3D12PipelineStateObject*>(pso);
		Context->GraphicsCmdList->SetPipelineState(dx12Pso->GetPipelineState());
		Context->GraphicsCmdList->RSSetViewports(1, &FrameBuffer->GetViewport());
		Context->GraphicsCmdList->RSSetScissorRects(1, &FrameBuffer->GetScissorsRect());

		// Indicate there will be a transition made to the resource.
		Context->GraphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
				FrameBuffer->CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			)
		);

		Context->GraphicsCmdList->OMSetRenderTargets(1, &FrameBuffer->GetCurrentBackBufferViewCpu(),
			true,
			&FrameBuffer->GetDepthStencilViewCpu()
		);
		ID3D12DescriptorHeap* descriptorHeaps[] = { Context->CbvHeap.Get() };
		Context->GraphicsCmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	}



	void D3D12RenderingApi::DrawGeometry
	(
		const std::vector<RenderItem*>& renderItems,
		UINT currentFrameResourceIndex
	)
	{
		/* Bind the shader root signature */
		Context->GraphicsCmdList->SetGraphicsRootSignature(Context->RootSignature.Get());
		const D3D12_GPU_VIRTUAL_ADDRESS passBufferAddress = CurrentFrameResource->PassBuffer->Resource()->GetGPUVirtualAddress();
		Context->GraphicsCmdList->SetGraphicsRootConstantBufferView(2, passBufferAddress);

		// For each render item...
		for (auto& renderItem : renderItems)
		{
			const auto renderItemDerived = dynamic_cast<D3D12RenderItem*>(renderItem);
			const auto d3d12VertexBuffer  = dynamic_cast<D3D12VertexBuffer*>(renderItem->Geometry->VertexBuffer.get());
			const auto d3d12IndexBuffer   = dynamic_cast<D3D12IndexBuffer*>(renderItem->Geometry->IndexBuffer.get());

			Context->GraphicsCmdList->IASetVertexBuffers(0, 1, &d3d12VertexBuffer->GetVertexBufferView());
			Context->GraphicsCmdList->IASetIndexBuffer(&d3d12IndexBuffer->GetIndexBufferView());
			Context->GraphicsCmdList->IASetPrimitiveTopology(renderItemDerived->PrimitiveType);

			const UINT objConstBufferByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));
			const UINT matConstBufferByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(MaterialConstants));

			ID3D12Resource* objectConstantBuffer	= CurrentFrameResource->ConstantBuffer->Resource();
			ID3D12Resource* materialConstantBuffer	= CurrentFrameResource->MaterialBuffer->Resource();

			const D3D12_GPU_VIRTUAL_ADDRESS objConstBufferAddress = objectConstantBuffer->GetGPUVirtualAddress() + renderItem->ObjectConstantBufferIndex * objConstBufferByteSize;
			const D3D12_GPU_VIRTUAL_ADDRESS materialBufferAddress = materialConstantBuffer->GetGPUVirtualAddress() + renderItem->Material->GetBufferIndex() * matConstBufferByteSize;

			Context->GraphicsCmdList->SetGraphicsRootConstantBufferView(0, objConstBufferAddress);
			Context->GraphicsCmdList->SetGraphicsRootConstantBufferView(1, materialBufferAddress);

			Context->GraphicsCmdList->DrawIndexedInstanced
			(
				renderItem->Geometry->IndexBuffer->GetCount(),
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
		Context->GraphicsCmdList->SetDescriptorHeaps(1, ImGuiImplD3D12::ImGuiHeap.GetAddressOf());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), Context->GraphicsCmdList.Get());
#endif

		// Now instruct we have made the changes to the buffer
		Context->GraphicsCmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				FrameBuffer->CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT
			)
		);


		THROW_ON_FAILURE(Context->GraphicsCmdList->Close());
		ID3D12CommandList* cmdsLists[] = { Context->GraphicsCmdList.Get() };
		Context->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

#ifdef ENGINE_IMGUI_SUPPORT
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, (void*)Context->GraphicsCmdList.Get());
		}
#endif
	
		THROW_ON_FAILURE(Context->SwapChain->Present(0, 0));
		FrameBuffer->SetBackBufferIndex((FrameBuffer->GetBackBufferIndex() + 1) % SWAP_CHAIN_BUFFER_COUNT);
		CurrentFrameResource->Fence = ++Context->GPU_TO_CPU_SYNC_COUNT;
		/**
		 * Add an instruction to the command queue to set a new fence point. 
		 * Because we are on the GPU timeline, the new fence point won't be 
		 * set until the GPU finishes processing all the commands prior to this Signal().
		 */
		Context->CommandQueue->Signal(Context->Fence.Get(), Context->GPU_TO_CPU_SYNC_COUNT);
	}

}
