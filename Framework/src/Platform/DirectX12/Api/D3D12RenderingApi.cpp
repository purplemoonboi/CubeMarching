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
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"
#include "Platform/DirectX12/RootSignature/D3D12RootSignature.h"
#include "Platform/DirectX12/Textures/Loader/D3D12TextureLoader.h"

#include <imgui.h>
#include <backends/imgui_impl_dx12.h>



namespace Engine
{

#define ENGINE_IMGUI_SUPPORT 1

	D3D12RenderingApi::~D3D12RenderingApi()
	{
	}

	void D3D12RenderingApi::Init(GraphicsContext* context, INT32 viewportWidth, INT32 viewportHeight)
	{
		Context = dynamic_cast<D3D12Context*>(context);

		D3D12HeapManager = CreateScope<class D3D12HeapManager>(Context->Device.Get());
		const HRESULT heapResult = D3D12HeapManager->Init(NUMBER_OF_FRAME_RESOURCES, 64);
		THROW_ON_FAILURE(heapResult);


		D3D12TextureLoader::Init(Context);
		D3D12BufferUtils::Init(Context->Device.Get(), Context->ResourceCommandList.Get());
		D3D12Utils::Init(D3D12HeapManager.get(), Context);

		D3D12CopyContext::Init(Context);

		FrameBufferSpecifications fbs;
		fbs.Width = viewportWidth;
		fbs.Height = viewportHeight;
		

		FrameBuffer = CreateScope<class D3D12FrameBuffer>(fbs);
		FrameBuffer->Init(Context);
		FrameBuffer->RebuildFrameBuffer(fbs);

		INT32 i;
		for(i = 0; i < 4; ++i)
		{
			RenderTargets[i] = CreateScope<D3D12RenderTarget>(nullptr, viewportWidth, viewportHeight);
		}

		for (i = 0; i < NUMBER_OF_FRAME_RESOURCES; ++i)
		{
			FrameResources.push_back(CreateScope<D3D12FrameResource>(Context, 1, 16, 64, 1));
		}

		UploadBuffer = CreateScope<D3D12ResourceBuffer>
		(
			Context->Device.Get(),
			D3D12HeapManager.get(),
			FrameResources,
			2
		);

	}

	void D3D12RenderingApi::SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		if(Context != nullptr && FrameBuffer != nullptr)
		{
			CORE_TRACE("Buffer resize");
			FrameBufferSpecifications fbSpecs = {};
			fbSpecs.Width = width;
			fbSpecs.Height = height;
			fbSpecs.OffsetX = x;
			fbSpecs.OffsetY = y;
			FrameBuffer->RebuildFrameBuffer(fbSpecs);
		}
	}

	void D3D12RenderingApi::PreInit()
	{
		// Reset the command allocator
		// Reset the command list
		THROW_ON_FAILURE(Context->ResourceCommandList->Reset
		(
			Context->ResourceAlloc.Get(),
			nullptr
		));
	}


	void D3D12RenderingApi::PostInit()
	{
		// Execute the initialization commands.
		HRESULT hr = Context->ResourceCommandList->Close();
		THROW_ON_FAILURE(hr);
		ID3D12CommandList* cmdsLists[] = { Context->ResourceCommandList.Get() };
		Context->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
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

		CurrentFrameResourceIndex = (CurrentFrameResourceIndex + 1) % NUMBER_OF_FRAME_RESOURCES;
		CurrentFrameResource = FrameResources[CurrentFrameResourceIndex].get();
		CORE_ASSERT(CurrentFrameResource, "No valid frame resource!");

		// Has the GPU finished processing the commands of the current frame resource
		// If not, wait until the GPU has completed commands up to this fence point.
		auto val = Context->Fence->GetCompletedValue();
		if (val < CurrentFrameResource->Fence)
		{
			const HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			const HRESULT eventCompletion = Context->Fence->SetEventOnCompletion(CurrentFrameResource->Fence, eventHandle);
			THROW_ON_FAILURE(eventCompletion);
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}

		// Update this buffer with respect to the
		// current frame resource. This is to avoid updating the vertex buffer if its in 'flight'
		if(terrain != nullptr)
		{
			//...check the buffer sizes align. Note, they always should unless we have
			//requested to regenerate the buffer.
			if (CurrentFrameResource->QueryTerrainBuffer(terrain->Geometry->VertexBuffer->GetCount()))
			{
				CurrentFrameResource->UpdateVoxelBuffer(Context, terrain->Geometry->VertexBuffer->GetCount());
			}

			if (terrain->NumFramesDirty > 0)
			{
				//...if so, we'll need to update the GPU buffer to point to the updated dynamic buffer
				UploadBuffer->UpdateVoxelTerrain(CurrentFrameResource, terrain);
			}
		}

		// Update constant buffers for each render item and material
		UploadBuffer->UpdateObjectBuffers(CurrentFrameResource, items);
		UploadBuffer->UpdateMaterialBuffers(CurrentFrameResource, materials);
		UploadBuffer->UpdatePassBuffer(CurrentFrameResource, settings, camera, deltaTime, elapsedTime, wireframe);

		// If everything checks out prepare recording instructions under the
		// current frame resource.
		CORE_ASSERT(Context->Device, "Device lost");
		CORE_ASSERT(CurrentFrameResource->GraphicsCommandList, "Invalid command list...");
		CORE_ASSERT(Context->CommandQueue, "Command queue lost");

		// Check for any scheduled buffer regenerations
		HRESULT hr = CurrentFrameResource->CommandAlloc->Reset();
		THROW_ON_FAILURE(hr);
		hr = CurrentFrameResource->GraphicsCommandList->Reset(CurrentFrameResource->CommandAlloc.Get(), nullptr);
		THROW_ON_FAILURE(hr);


		for(auto& rt : RenderTargets)
		{
			if(rt->DirtyFlag == 1)
			{
				rt->Regenerate();
			}
		}

		const HRESULT closeResult = CurrentFrameResource->GraphicsCommandList->Close();
		THROW_ON_FAILURE(closeResult);
		ID3D12CommandList* cmdsLists[] = { CurrentFrameResource->GraphicsCommandList.Get() };
		Context->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		CurrentFrameResource->Fence = ++Context->SyncCounter;

		const HRESULT signalResult = Context->CommandQueue->Signal(Context->Fence.Get(), CurrentFrameResource->Fence);
		THROW_ON_FAILURE(signalResult);

		if(Context->Fence->GetCompletedValue() < CurrentFrameResource->Fence)
		{
			const HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			const HRESULT eventCompletion = Context->Fence->SetEventOnCompletion(CurrentFrameResource->Fence, eventHandle);
			THROW_ON_FAILURE(eventCompletion);
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
		
		

		

	}

	void D3D12RenderingApi::PostRender()
	{}

	void D3D12RenderingApi::OnBeginRender()
	{
		HRESULT hr = CurrentFrameResource->CommandAlloc->Reset();
		THROW_ON_FAILURE(hr);
		hr = CurrentFrameResource->GraphicsCommandList->Reset(CurrentFrameResource->CommandAlloc.Get(), nullptr);
		THROW_ON_FAILURE(hr);

		CD3DX12_RESOURCE_BARRIER rtBarriers[4] = {  };


		for(INT32 i = 0; i < 4; ++i)
		{
			rtBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[i]->GpuResource.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		CurrentFrameResource->GraphicsCommandList->ResourceBarrier(4, &rtBarriers[0]);


		/* Bind the shader root signature */
		CurrentFrameResource->GraphicsCommandList->SetGraphicsRootSignature(RootSignature.Get());

		ID3D12DescriptorHeap* descriptorHeaps[] = { D3D12HeapManager->GetShaderResourceDescHeap() };
		CurrentFrameResource->GraphicsCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		const D3D12_GPU_VIRTUAL_ADDRESS passBufferAddress = CurrentFrameResource->PassBuffer->Resource()->GetGPUVirtualAddress();
		CurrentFrameResource->GraphicsCommandList->SetGraphicsRootConstantBufferView(2, passBufferAddress);

		// We can bind all textures in the scene - we declared 'n' amount of descriptors in the root signature.
		CurrentFrameResource->GraphicsCommandList->SetGraphicsRootDescriptorTable(3,
			D3D12HeapManager->GetShaderResourceDescHeap()->GetGPUDescriptorHandleForHeapStart());
	}

	void D3D12RenderingApi::OnEndRender()
	{
		CD3DX12_RESOURCE_BARRIER rtBarriers[4] = {  };

		for (INT32 i = 0; i < 4; ++i)
		{
			rtBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[i]->GpuResource.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		}

		CurrentFrameResource->GraphicsCommandList->ResourceBarrier(4, &rtBarriers[0]);
	}



	void D3D12RenderingApi::BindDepthPass()
	{
		CurrentFrameResource->GraphicsCommandList->RSSetViewports(1, &RenderTargets[(INT8)RenderLayer::Depth]->Viewport);
		CurrentFrameResource->GraphicsCommandList->RSSetScissorRects(1, &RenderTargets[(INT8)RenderLayer::Depth]->Rect);

		// Specify the buffers we are going to render to.
		CurrentFrameResource->GraphicsCommandList->OMSetRenderTargets(1, &RenderTargets[(INT8)RenderLayer::Depth]->ResourceCpuRtv,
			true, &FrameBuffer->GetDepthStencilViewCpu());

		// Clear the back buffer and depth buffer.
		CurrentFrameResource->GraphicsCommandList->ClearRenderTargetView(RenderTargets[(INT8)RenderLayer::Depth]->ResourceCpuRtv, DirectX::Colors::LightBlue, 0, nullptr);
		CurrentFrameResource->GraphicsCommandList->ClearDepthStencilView(FrameBuffer->GetDepthStencilViewCpu(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		//CurrentFrameResource->GraphicsCommandList->SetPipelineState();
	}

	void D3D12RenderingApi::BindScenePass()
	{
		CurrentFrameResource->GraphicsCommandList->RSSetViewports(1, &RenderTargets[(INT8)RenderLayer::Albedo]->Viewport);
		CurrentFrameResource->GraphicsCommandList->RSSetScissorRects(1, &RenderTargets[(INT8)RenderLayer::Albedo]->Rect);

		// Specify the buffers we are going to render to.
		CurrentFrameResource->GraphicsCommandList->OMSetRenderTargets(1, &RenderTargets[(INT8)RenderLayer::Albedo]->ResourceCpuRtv,
			true, &FrameBuffer->GetDepthStencilViewCpu());

		// Clear the back buffer and depth buffer.
		CurrentFrameResource->GraphicsCommandList->ClearRenderTargetView(RenderTargets[(INT8)RenderLayer::Albedo]->ResourceCpuRtv, DirectX::Colors::LightBlue, 0, nullptr);
		CurrentFrameResource->GraphicsCommandList->ClearDepthStencilView(FrameBuffer->GetDepthStencilViewCpu(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		//CurrentFrameResource->GraphicsCommandList->SetPipelineState();
	}

	void D3D12RenderingApi::BindLightingPass()
	{
		CurrentFrameResource->GraphicsCommandList->RSSetViewports(1, &RenderTargets[(INT8)RenderLayer::Lighting]->Viewport);
		CurrentFrameResource->GraphicsCommandList->RSSetScissorRects(1, &RenderTargets[(INT8)RenderLayer::Lighting]->Rect);

		// Specify the buffers we are going to render to.
		CurrentFrameResource->GraphicsCommandList->OMSetRenderTargets(1, &RenderTargets[(INT8)RenderLayer::Lighting]->ResourceCpuRtv,
			true, &FrameBuffer->GetDepthStencilViewCpu());

		// Clear the back buffer and depth buffer.
		CurrentFrameResource->GraphicsCommandList->ClearRenderTargetView(RenderTargets[(INT8)RenderLayer::Lighting]->ResourceCpuRtv, DirectX::Colors::LightBlue, 0, nullptr);
		CurrentFrameResource->GraphicsCommandList->ClearDepthStencilView(FrameBuffer->GetDepthStencilViewCpu(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		//CurrentFrameResource->GraphicsCommandList->SetPipelineState();
	}

	void D3D12RenderingApi::BindPostProcessingPass()
	{

	}

	void D3D12RenderingApi::DrawTerrainGeometry(PipelineStateObject* pso, RenderItem* terrain)
	{
		const auto dx12Pso = dynamic_cast<D3D12PipelineStateObject*>(pso);
		CurrentFrameResource->GraphicsCommandList->SetPipelineState(dx12Pso->GetPipelineState());


		if (terrain != nullptr && CurrentFrameResource != nullptr)
		{

			const auto renderItemDerived = dynamic_cast<D3D12RenderItem*>(terrain);
			const auto d3d12VertexBuffer = dynamic_cast<D3D12VertexBuffer*>(terrain->Geometry->VertexBuffer.get());
			const auto d3d12IndexBuffer = dynamic_cast<D3D12IndexBuffer*>(terrain->Geometry->IndexBuffer.get());

			CurrentFrameResource->GraphicsCommandList->IASetVertexBuffers(0, 1, &d3d12VertexBuffer->GetVertexBufferView());
			CurrentFrameResource->GraphicsCommandList->IASetIndexBuffer(&d3d12IndexBuffer->GetIndexBufferView());
			CurrentFrameResource->GraphicsCommandList->IASetPrimitiveTopology(renderItemDerived->PrimitiveType);

			const UINT objConstBufferByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));
			const UINT matConstBufferByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(MaterialConstants));

			ID3D12Resource* objectConstantBuffer = CurrentFrameResource->ConstantBuffer->Resource();
			ID3D12Resource* materialConstantBuffer = CurrentFrameResource->MaterialBuffer->Resource();

			const D3D12_GPU_VIRTUAL_ADDRESS objConstBufferAddress = objectConstantBuffer->GetGPUVirtualAddress() + renderItemDerived->ObjectConstantBufferIndex * objConstBufferByteSize;
			const D3D12_GPU_VIRTUAL_ADDRESS materialBufferAddress = materialConstantBuffer->GetGPUVirtualAddress() + renderItemDerived->Material->GetMaterialIndex() * matConstBufferByteSize;

			CurrentFrameResource->GraphicsCommandList->SetGraphicsRootConstantBufferView(0, objConstBufferAddress);
			CurrentFrameResource->GraphicsCommandList->SetGraphicsRootConstantBufferView(1, materialBufferAddress);


			CurrentFrameResource->GraphicsCommandList->DrawInstanced
			(
				terrain->Geometry->VertexBuffer->GetCount(),
				1,
				terrain->StartIndexLocation,
				terrain->BaseVertexLocation
			);
		}

	}


	void D3D12RenderingApi::DrawSceneStaticGeometry(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems)
	{
		const auto dx12Pso = dynamic_cast<D3D12PipelineStateObject*>(pso);
		CurrentFrameResource->GraphicsCommandList->SetPipelineState(dx12Pso->GetPipelineState());
		
		
		// For each render item...
		for (auto& renderItem : renderItems)
		{


			const auto renderItemDerived = dynamic_cast<D3D12RenderItem*>(renderItem);
			const auto d3d12VertexBuffer = dynamic_cast<D3D12VertexBuffer*>(renderItem->Geometry->VertexBuffer.get());
			const auto d3d12IndexBuffer = dynamic_cast<D3D12IndexBuffer*>(renderItem->Geometry->IndexBuffer.get());

			CurrentFrameResource->GraphicsCommandList->IASetVertexBuffers(0, 1, &d3d12VertexBuffer->GetVertexBufferView());
			CurrentFrameResource->GraphicsCommandList->IASetIndexBuffer(&d3d12IndexBuffer->GetIndexBufferView());
			CurrentFrameResource->GraphicsCommandList->IASetPrimitiveTopology(renderItemDerived->PrimitiveType);

			const UINT objConstBufferByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));
			const UINT matConstBufferByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(MaterialConstants));

			ID3D12Resource* objectConstantBuffer = CurrentFrameResource->ConstantBuffer->Resource();
			ID3D12Resource* materialConstantBuffer = CurrentFrameResource->MaterialBuffer->Resource();

			const D3D12_GPU_VIRTUAL_ADDRESS objConstBufferAddress = objectConstantBuffer->GetGPUVirtualAddress() + renderItem->ObjectConstantBufferIndex * objConstBufferByteSize;
			const D3D12_GPU_VIRTUAL_ADDRESS materialBufferAddress = materialConstantBuffer->GetGPUVirtualAddress() + renderItem->Material->GetMaterialIndex() * matConstBufferByteSize;

			CurrentFrameResource->GraphicsCommandList->SetGraphicsRootConstantBufferView(0, objConstBufferAddress);
			CurrentFrameResource->GraphicsCommandList->SetGraphicsRootConstantBufferView(1, materialBufferAddress);


			CurrentFrameResource->GraphicsCommandList->DrawIndexedInstanced
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

		const HRESULT closeResult = CurrentFrameResource->GraphicsCommandList->Close();
		THROW_ON_FAILURE(closeResult);

		ID3D12CommandList* cmdList[] = { CurrentFrameResource->GraphicsCommandList.Get() };
		Context->CommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

		CurrentFrameResource->Fence = ++Context->SyncCounter;
		Context->CommandQueue->Signal(Context->Fence.Get(), CurrentFrameResource->Fence);
	}



}
