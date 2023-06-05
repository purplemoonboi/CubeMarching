#include "D3D12RenderingApi.h"
#include "Framework/Core/Log/Log.h"
#include "Framework/ImGui/ImGuiApi/ImGuiInstructions.h"



#include "Platform/Directx12/Buffers/D3D12FrameBuffer.h"
#include "Platform/Directx12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/RenderItems/D3D12RenderItem.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"
#include "Platform/DirectX12/Copy/D3D12CopyContext.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Platform/DirectX12/Utilities/D3D12BufferFactory.h"
#include "Platform/DirectX12/RootSignature/D3D12RootSignature.h"
#include "Platform/DirectX12/Textures/Loader/D3D12TextureLoader.h"

#include <imgui.h>
#include <backends/imgui_impl_dx12.h>



namespace Foundation
{
	D3D12RenderingApi::D3D12RenderingApi()
	{
	}

	D3D12RenderingApi::~D3D12RenderingApi(){}

	void D3D12RenderingApi::Init(GraphicsContext* context, INT32 viewportWidth, INT32 viewportHeight)
	{
		HRESULT hr{ S_OK };
		Context = dynamic_cast<D3D12Context*>(context);

		HeapManager = CreateScope<D3D12HeapManager>(Context->pDevice.Get());

		hr = HeapManager->Init(FRAMES_IN_FLIGHT, 32);
		THROW_ON_FAILURE(hr);

		D3D12TextureLoader::Init(Context);
		D3D12BufferFactory::Init(Context->pDevice.Get(), Context->pGCL.Get());
		D3D12ResourceFactory::Init(HeapManager.get(), Context);

		FrameBufferSpecifications fbs;
		fbs.Width = viewportWidth;
		fbs.Height = viewportHeight;
		
		FrameBuffer = CreateScope<D3D12FrameBuffer>(fbs);
		FrameBuffer->Init(Context, fbs);

		INT32 i;
		for(i = 0; i < 4; ++i)
		{
			RenderTargets[i] = CreateScope<D3D12RenderTarget>(nullptr, 1920, 1080);
		}

		for (i = 0; i < FRAMES_IN_FLIGHT; ++i)
		{
			Frames[i] = CreateScope<D3D12FrameResource>(Context, 1, 16, 64, 1);
		}

		UploadBuffer = CreateScope<D3D12ResourceBuffer>(Context->pDevice.Get(), HeapManager.get(), Frames, 2);

	}

	void D3D12RenderingApi::Clean()
	{
	}

	void D3D12RenderingApi::SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		if(Context != nullptr)
		{
			CORE_TRACE("Buffer resize w: {0}, h: {1}", width, height);
			FrameBufferSpecifications fbSpecs = {};
			fbSpecs.Width = width;
			fbSpecs.Height = height;
			fbSpecs.OffsetX = x;
			fbSpecs.OffsetY = y;
			FrameBuffer->OnResizeFrameBuffer(fbSpecs);
		}
	}

	void D3D12RenderingApi::PreInit()
	{
		HRESULT hr{ S_OK };
		hr = Context->pGCL->Reset(Context->pCmdAlloc.Get(), nullptr);
		THROW_ON_FAILURE(hr);
	}


	void D3D12RenderingApi::PostInit()
	{
		// Execute the initialization commands.
		HRESULT hr = Context->pGCL->Close();
		THROW_ON_FAILURE(hr);
		ID3D12CommandList* pList[] = { Context->pGCL.Get() };
		Context->pQueue->ExecuteCommandLists(_countof(pList), pList);
		Context->FlushCommandQueue();
	}

	void D3D12RenderingApi::PreRender(
		const std::vector<RenderItem*>& items, const std::vector<Material*>& materials,
		RenderItem* terrain,
		const WorldSettings& settings,
		const MainCamera& camera,
		float deltaTime,
		float elapsedTime,
		bool wireframe
	)
	{
		HRESULT hr{ S_OK };
		FrameIndex = (FrameIndex + 1) % FRAMES_IN_FLIGHT;

		// Has the GPU finished processing the commands of the current frame resource
		// If not, wait until the GPU has completed commands up to this fence point.
		if (Context->pFence->GetCompletedValue() < Frames[FrameIndex]->Fence)
		{
			const HANDLE pEvent = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			const HRESULT pEventState = Context->pFence->SetEventOnCompletion(Frames[FrameIndex]->Fence, pEvent);
			THROW_ON_FAILURE(pEventState);
			WaitForSingleObject(pEvent, INFINITE);
			CloseHandle(pEvent);
		}

		// Update this buffer with respect to the
		// current frame resource. This is to avoid updating the vertex buffer if its in 'flight'
		if(terrain != nullptr)
		{
			//...check the buffer sizes align. Note, they always should unless we have
			//requested to regenerate the buffer.
			if (Frames[FrameIndex]->QueryTerrainBuffer(terrain->Geometry->VertexBuffer->GetCount()))
			{
				Frames[FrameIndex]->UpdateVoxelBuffer(Context, terrain->Geometry->VertexBuffer->GetCount());
			}

			if (terrain->NumFramesDirty > 0)
			{
				//...if so, we'll need to update the GPU buffer to point to the updated dynamic buffer
				UploadBuffer->UpdateVoxelTerrain(Frames[FrameIndex].get(), terrain);
			}
		}

		// Update constant buffers for each render item and material
		UploadBuffer->UpdateObjectBuffers(Frames[FrameIndex].get(), items);
		UploadBuffer->UpdateMaterialBuffers(Frames[FrameIndex].get(), materials);
		UploadBuffer->UpdatePassBuffer(Frames[FrameIndex].get(), settings, camera, deltaTime, elapsedTime, wireframe);

		// If everything checks out prepare recording instructions under the
		// current frame resource.
		CORE_ASSERT(Context->pDevice, "Device lost");
		CORE_ASSERT(Frames[FrameIndex]->pGCL, "Invalid command list...");
		CORE_ASSERT(Context->pQueue, "Invalid command queue...");

		// Check for any scheduled buffer regenerations
		hr = Frames[FrameIndex]->pCmdAlloc->Reset();
		THROW_ON_FAILURE(hr);
		hr = Frames[FrameIndex]->pGCL->Reset(Frames[FrameIndex]->pCmdAlloc.Get(), nullptr);
		THROW_ON_FAILURE(hr);


		for(auto& rt : RenderTargets)
		{
			if(rt->DirtyFlag == 1)
			{
				rt->Regenerate();
			}
		}

		hr = Frames[FrameIndex]->pGCL->Close();
		THROW_ON_FAILURE(hr);
		ID3D12CommandList* cmdsLists[] = { Frames[FrameIndex]->pGCL.Get() };
		Context->pQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		Frames[FrameIndex]->Fence = ++Context->SyncCounter;

		hr = Context->pQueue->Signal(Context->pFence.Get(), Frames[FrameIndex]->Fence);
		THROW_ON_FAILURE(hr);

		if(Context->pFence->GetCompletedValue() < Frames[FrameIndex]->Fence)
		{
			const HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			const HRESULT eventCompletion = Context->pFence->SetEventOnCompletion(Frames[FrameIndex]->Fence, eventHandle);
			THROW_ON_FAILURE(eventCompletion);
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	void D3D12RenderingApi::PostRender()
	{}

	void D3D12RenderingApi::OnBeginRender()
	{
		THROW_ON_FAILURE(Frames[FrameIndex]->pCmdAlloc->Reset());
		THROW_ON_FAILURE(Frames[FrameIndex]->pGCL->Reset(Frames[FrameIndex]->pCmdAlloc.Get(), nullptr));
		CD3DX12_RESOURCE_BARRIER rtBarriers[4] = {  };

		for(INT32 i = 0; i < 4; ++i)
		{
			rtBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[i]->pResource.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		// Bind core render root signature.
		// Bind the resource descriptor heap and keep it live until the end of the passes.
		Frames[FrameIndex]->pGCL->ResourceBarrier(4, &rtBarriers[0]);
		Frames[FrameIndex]->pGCL->SetGraphicsRootSignature(RootSignature.Get());

		ID3D12DescriptorHeap* descriptorHeaps[] = { HeapManager->GetShaderResourceDescHeap() };
		Frames[FrameIndex]->pGCL->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		const D3D12_GPU_VIRTUAL_ADDRESS passBufferAddress = Frames[FrameIndex]->PassBuffer->Resource()->GetGPUVirtualAddress();
		Frames[FrameIndex]->pGCL->SetGraphicsRootConstantBufferView(2, passBufferAddress);

		// We can bind all textures in the scene - we declared 'n' amount of descriptors in the root signature.
		Frames[FrameIndex]->pGCL->SetGraphicsRootDescriptorTable(3, HeapManager->GetShaderResourceDescHeap()->GetGPUDescriptorHandleForHeapStart());
	}

	void D3D12RenderingApi::OnEndRender()
	{
		CD3DX12_RESOURCE_BARRIER rtBarriers[4] = {  };

		for (INT32 i = 0; i < 4; ++i)
		{
			rtBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[i]->pResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		Frames[FrameIndex]->pGCL->ResourceBarrier(4, &rtBarriers[0]);
	}

	void D3D12RenderingApi::BindPasses()
	{
		/*
		Frames[FrameIndex]->pGCL->RSSetViewports(1, &RenderTargets[(INT8)RenderLayer::Albedo].Viewport);
		Frames[FrameIndex]->pGCL->RSSetScissorRects(1, &RenderTargets[(INT8)RenderLayer::Albedo].Rect);

		// Specify the buffers we are going to render to.
		Frames[FrameIndex]->pGCL->OMSetRenderTargets(1, &RenderTargets[(INT8)RenderLayer::Albedo].pRTV,
			true, &FrameBuffer.GetDepthStencilViewCpu());

		// Clear the back buffer and depth buffer.
		Frames[FrameIndex]->pGCL->ClearRenderTargetView(RenderTargets[(INT8)RenderLayer::Albedo].pRTV, DirectX::Colors::LightBlue, 0, nullptr);
		Frames[FrameIndex]->pGCL->ClearDepthStencilView(FrameBuffer.GetDepthStencilViewCpu(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		//Frames[FrameIndex]->pGCL->SetPipelineState();
		*/

		const float clearColour[]{ 0.f, 0.f, 0.f, 0.f };
		CD3DX12_CLEAR_VALUE clearValue{ DXGI_FORMAT_R32G32B32_FLOAT, clearColour };

		D3D12_RENDER_PASS_BEGINNING_ACCESS renderPassBeginningAccessClear{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR, { clearValue } };
		D3D12_RENDER_PASS_ENDING_ACCESS renderPassEndingAccessPreserve{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, {} };
		D3D12_RENDER_PASS_RENDER_TARGET_DESC renderPassRenderTargetDesc
		{
			HeapManager->GetRenderTargetHeap()->GetCPUDescriptorHandleForHeapStart(),
			renderPassBeginningAccessClear, renderPassEndingAccessPreserve
		};

		D3D12_RENDER_PASS_BEGINNING_ACCESS renderPassBeginningAccessNoAccess{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS, {} };
		D3D12_RENDER_PASS_ENDING_ACCESS renderPassEndingAccessNoAccess{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS, {} };
		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC renderPassDepthStencilDesc
		{
			HeapManager->GetDepthStencilHeap()->GetCPUDescriptorHandleForHeapStart(),
			renderPassBeginningAccessNoAccess, renderPassBeginningAccessNoAccess,
			renderPassEndingAccessNoAccess, renderPassEndingAccessNoAccess
		};

		//Albedo Pass
		Frames[FrameIndex]->pGCL->BeginRenderPass(1, &renderPassRenderTargetDesc, &renderPassDepthStencilDesc, D3D12_RENDER_PASS_FLAG_NONE);
		// Record Commands

		//Frames[FrameIndex]->pGCL->SetPipelineState1();


		Frames[FrameIndex]->pGCL->EndRenderPass();
	}

	void D3D12RenderingApi::DrawTerrainGeometry(PipelineStateObject* pso, RenderItem* terrain)
	{
		const auto dx12Pso = dynamic_cast<D3D12PipelineStateObject*>(pso);
		Frames[FrameIndex]->pGCL->SetPipelineState(dx12Pso->GetPipelineState());

		const auto renderItemDerived = dynamic_cast<D3D12RenderItem*>(terrain);
		const auto d3d12VertexBuffer = dynamic_cast<D3D12VertexBuffer*>(terrain->Geometry->VertexBuffer.get());
		const auto d3d12IndexBuffer = dynamic_cast<D3D12IndexBuffer*>(terrain->Geometry->IndexBuffer.get());

		Frames[FrameIndex]->pGCL->IASetVertexBuffers(0, 1, &d3d12VertexBuffer->GetVertexBufferView());
		Frames[FrameIndex]->pGCL->IASetIndexBuffer(&d3d12IndexBuffer->GetIndexBufferView());
		Frames[FrameIndex]->pGCL->IASetPrimitiveTopology(renderItemDerived->PrimitiveType);

		const UINT objConstBufferByteSize = D3D12BufferFactory::CalculateBufferByteSize(sizeof(ObjectConstant));
		const UINT matConstBufferByteSize = D3D12BufferFactory::CalculateBufferByteSize(sizeof(MaterialConstants));

		ID3D12Resource* objectConstantBuffer = Frames[FrameIndex]->ConstantBuffer->Resource();
		ID3D12Resource* materialConstantBuffer = Frames[FrameIndex]->MaterialBuffer->Resource();

		const D3D12_GPU_VIRTUAL_ADDRESS objConstBufferAddress = objectConstantBuffer->GetGPUVirtualAddress() + renderItemDerived->ObjectConstantBufferIndex * objConstBufferByteSize;
		const D3D12_GPU_VIRTUAL_ADDRESS materialBufferAddress = materialConstantBuffer->GetGPUVirtualAddress() + renderItemDerived->Material->GetMaterialIndex() * matConstBufferByteSize;

		Frames[FrameIndex]->pGCL->SetGraphicsRootConstantBufferView(0, objConstBufferAddress);
		Frames[FrameIndex]->pGCL->SetGraphicsRootConstantBufferView(1, materialBufferAddress);

		Frames[FrameIndex]->pGCL->DrawInstanced(terrain->Geometry->VertexBuffer->GetCount(), 1, terrain->StartIndexLocation, terrain->BaseVertexLocation);
		

	}


	void D3D12RenderingApi::DrawSceneStaticGeometry(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems)
	{
		const auto dx12Pso = dynamic_cast<D3D12PipelineStateObject*>(pso);
		Frames[FrameIndex]->pGCL->SetPipelineState(dx12Pso->GetPipelineState());
		
		// For each render item...
		for (auto& renderItem : renderItems)
		{

			const auto renderItemDerived = dynamic_cast<D3D12RenderItem*>(renderItem);
			const auto d3d12VertexBuffer = dynamic_cast<D3D12VertexBuffer*>(renderItem->Geometry->VertexBuffer.get());
			const auto d3d12IndexBuffer = dynamic_cast<D3D12IndexBuffer*>(renderItem->Geometry->IndexBuffer.get());

			Frames[FrameIndex]->pGCL->IASetVertexBuffers(0, 1, &d3d12VertexBuffer->GetVertexBufferView());
			Frames[FrameIndex]->pGCL->IASetIndexBuffer(&d3d12IndexBuffer->GetIndexBufferView());
			Frames[FrameIndex]->pGCL->IASetPrimitiveTopology(renderItemDerived->PrimitiveType);

			const UINT objConstBufferByteSize = D3D12BufferFactory::CalculateBufferByteSize(sizeof(ObjectConstant));
			const UINT matConstBufferByteSize = D3D12BufferFactory::CalculateBufferByteSize(sizeof(MaterialConstants));

			ID3D12Resource* objectConstantBuffer = Frames[FrameIndex]->ConstantBuffer->Resource();
			ID3D12Resource* materialConstantBuffer = Frames[FrameIndex]->MaterialBuffer->Resource();

			const D3D12_GPU_VIRTUAL_ADDRESS objConstBufferAddress = objectConstantBuffer->GetGPUVirtualAddress() + renderItem->ObjectConstantBufferIndex * objConstBufferByteSize;
			const D3D12_GPU_VIRTUAL_ADDRESS materialBufferAddress = materialConstantBuffer->GetGPUVirtualAddress() + renderItem->Material->GetMaterialIndex() * matConstBufferByteSize;

			Frames[FrameIndex]->pGCL->SetGraphicsRootConstantBufferView(0, objConstBufferAddress);
			Frames[FrameIndex]->pGCL->SetGraphicsRootConstantBufferView(1, materialBufferAddress);
			Frames[FrameIndex]->pGCL->DrawIndexedInstanced(renderItem->Geometry->IndexBuffer->GetCount(), 1, renderItem->StartIndexLocation, renderItem->BaseVertexLocation, 0);

		}
	}

	void D3D12RenderingApi::Flush()
	{
		HRESULT hr{ S_OK };
		hr = Frames[FrameIndex]->pGCL->Close();
		THROW_ON_FAILURE(hr);
		ID3D12CommandList* cmdList[] = { Frames[FrameIndex]->pGCL.Get() };
		Context->pQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
		Frames[FrameIndex]->Fence = ++Context->SyncCounter;
		hr = Context->pQueue->Signal(Context->pFence.Get(), Frames[FrameIndex]->Fence);
		THROW_ON_FAILURE(hr);
	}

}
