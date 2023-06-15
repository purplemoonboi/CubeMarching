#include "D3D12RenderingApi.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Core/D3D12Core.h"
#include "Platform/Directx12/Buffers/D3D12FrameBuffer.h"
#include "Platform/Directx12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/RenderItems/D3D12RenderItem.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"
#include "Platform/DirectX12/Heap/D3D12HeapManager.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineResource.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Platform/DirectX12/Utilities/D3D12BufferFactory.h"
#include "Platform/DirectX12/Textures/D3D12RenderTarget.h"
#include "Platform/DirectX12/Textures/Loader/D3D12TextureLoader.h"

#include "Framework/Scene/Scene.h"


namespace Foundation::Graphics::D3D12
{


	void D3D12RenderingPipelines::Insert(const std::string& name, D3D12PipelineStateObject* pso)
	{
		if(pPipelineStates.find(name) == pPipelineStates.end())
		{
			pPipelineStates.emplace(name, pso);
		}
		else
		{
			CORE_WARNING("Pipeline state {0} already exists!", name);
			APP_WARNING("Pipeline state {0} already exists!", name);
		}
	}

	void D3D12RenderingPipelines::InsertStateObject(const std::string& name, ID3D12StateObject* pso)
	{
		if (pPipelineObjects.find(name) == pPipelineObjects.end())
		{
			pPipelineObjects.emplace(name, pso);
		}
		else
		{
			CORE_WARNING("State object {0} already exists!", name);
			APP_WARNING("State object {0} already exists!", name);
		}
	}

	void D3D12RenderingPipelines::Remove(const std::string& name)
	{
	}

	void D3D12RenderingPipelines::RemoveStateObject(const std::string& name)
	{
	}

	D3D12RenderingApi::D3D12RenderingApi()
	{
	}

	D3D12RenderingApi::~D3D12RenderingApi()
	{

		// Release heaps
		RtvHeap.Release();
		DsvHeap.Release();
		SrvHeap.Release();
		UavHeap.Release();

	}

	void D3D12RenderingApi::Init(GraphicsContext* context)
	{
		HRESULT hr{ S_OK };
		Context = dynamic_cast<D3D12Context*>(context);
		Context->Init();

		

		D3D12BufferFactory::Init(Context->pGCL.Get());
		D3D12TextureLoader::Init(Context->pGCL.Get());

	

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

	const FrameBufferSpecifications& D3D12RenderingApi::GetViewportSpecifications() const
	{
		return FrameBuffer->GetSpecifications();
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
		//Flush();
	}

	void D3D12RenderingApi::OnPreBeginRender
	(
		MainCamera* camera,
		AppTimeManager* time,
		const std::vector<RenderItem*>& items, 
		const std::vector<Material*>& materials,
		bool wireframe
	)
	{

		HRESULT hr{ S_OK };
		FrameIndex = (FrameIndex + 1) % FRAMES_IN_FLIGHT;
		CurrentFrameIndex = FrameIndex;

		// Has the GPU finished processing the commands of the current frame resource
		// If not, wait until the GPU has completed commands up to this fence point.
		if (Context->pFence->GetCompletedValue() < RenderFrames[FrameIndex].Fence)
		{
			const HANDLE pEvent = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			const HRESULT pEventState = Context->pFence->SetEventOnCompletion(RenderFrames[FrameIndex].Fence, pEvent);
			THROW_ON_FAILURE(pEventState);
			WaitForSingleObject(pEvent, INFINITE);
			CloseHandle(pEvent);
		}

		// Process pending free operations
		if(DeferralFlags[FrameIndex] == 1)
		{
			ProcessDeferrals(FrameIndex);
		}

		// Update constant buffers for each render item and material
		UploadBuffer->UpdateObjectBuffers(&RenderFrames[FrameIndex], items);
		UploadBuffer->UpdateMaterialBuffers(&RenderFrames[FrameIndex], materials);
		UploadBuffer->UpdatePassBuffer(&RenderFrames[FrameIndex], camera, time, wireframe);

	}

	void D3D12RenderingApi::OnBeginRender()
	{
		// If everything checks out prepare recording instructions under the
		// current frame resource.
		CORE_ASSERT(pDevice, "Device lost");
		CORE_ASSERT(Context->pQueue, "Invalid command queue...");
		CORE_ASSERT(RenderFrames[FrameIndex].pGCL, "Invalid command list...");

		THROW_ON_FAILURE(RenderFrames[FrameIndex].pCmdAlloc->Reset());
		THROW_ON_FAILURE(RenderFrames[FrameIndex].pGCL->Reset(RenderFrames[FrameIndex].pCmdAlloc.Get(), nullptr));
		CD3DX12_RESOURCE_BARRIER rtBarriers[GBUFFER_SIZE] = {  };

		for(INT32 i = 0; i < GBUFFER_SIZE; ++i)
		{
			rtBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[i]->pResource.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, 
				D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		// Bind core render root signature.
		// Bind the resource descriptor heap and keep it live until the end of the passes.
		RenderFrames[FrameIndex].pGCL->ResourceBarrier(GBUFFER_SIZE, &rtBarriers[0]);
		RenderFrames[FrameIndex].pGCL->SetGraphicsRootSignature(pRootSignature.Get());

		ID3D12DescriptorHeap* descriptorHeaps[] = { SrvHeap.GetHeap() };
		RenderFrames[FrameIndex].pGCL->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		const D3D12_GPU_VIRTUAL_ADDRESS passBufferAddress = RenderFrames[FrameIndex].PassBuffer.Resource()->GetGPUVirtualAddress();
		RenderFrames[FrameIndex].pGCL->SetGraphicsRootConstantBufferView(2, passBufferAddress);

		// We can bind all textures in the scene - we declared 'n' amount of descriptors in the root signature.
		RenderFrames[FrameIndex].pGCL->SetGraphicsRootDescriptorTable(3, SrvHeap.BeginGPU());

	}

	void D3D12RenderingApi::BindPasses()
	{
		/*
		RenderFrames[FrameIndex].pGCL->RSSetViewports(1, &RenderTargets[(INT8)RenderLayer::Albedo].Viewport);
		RenderFrames[FrameIndex].pGCL->RSSetScissorRects(1, &RenderTargets[(INT8)RenderLayer::Albedo].Rect);

		// Specify the buffers we are going to render to.
		RenderFrames[FrameIndex].pGCL->OMSetRenderTargets(1, &RenderTargets[(INT8)RenderLayer::Albedo].pRTV,
			true, &FrameBuffer.GetDepthStencilViewCpu());

		// Clear the back buffer and depth buffer.
		RenderFrames[FrameIndex].pGCL->ClearRenderTargetView(RenderTargets[(INT8)RenderLayer::Albedo].pRTV, DirectX::Colors::LightBlue, 0, nullptr);
		RenderFrames[FrameIndex].pGCL->ClearDepthStencilView(FrameBuffer.GetDepthStencilViewCpu(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		//RenderFrames[FrameIndex].pGCL->SetPipelineState();
		*/

		//const float clearColour[]{ 0.f, 0.f, 0.f, 0.f };
		//CD3DX12_CLEAR_VALUE clearValue{ DXGI_FORMAT_R32G32B32_FLOAT, clearColour };

		//D3D12_RENDER_PASS_BEGINNING_ACCESS renderPassBeginningAccessClear{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR, { clearValue } };
		//D3D12_RENDER_PASS_ENDING_ACCESS renderPassEndingAccessPreserve{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, {} };
		//D3D12_RENDER_PASS_RENDER_TARGET_DESC renderPassRenderTargetDesc
		//{
		//	HeapManager->GetRenderTargetHeap()->GetCPUDescriptorHandleForHeapStart(),
		//	renderPassBeginningAccessClear, renderPassEndingAccessPreserve
		//};

		//D3D12_RENDER_PASS_BEGINNING_ACCESS renderPassBeginningAccessNoAccess{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS, {} };
		//D3D12_RENDER_PASS_ENDING_ACCESS renderPassEndingAccessNoAccess{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS, {} };
		//D3D12_RENDER_PASS_DEPTH_STENCIL_DESC renderPassDepthStencilDesc
		//{
		//	HeapManager->GetDepthStencilHeap()->GetCPUDescriptorHandleForHeapStart(),
		//	renderPassBeginningAccessNoAccess, renderPassBeginningAccessNoAccess,
		//	renderPassEndingAccessNoAccess, renderPassEndingAccessNoAccess
		//};

		//// Bind render target
		//RenderFrames[FrameIndex].pGCL->OMSetRenderTargets(1, &RenderTargets[static_cast<UINT32>(RenderLayer::Albedo)]->pRTV, 
		//	FALSE,
		//	&RenderTargets[static_cast<UINT32>(RenderLayer::Albedo)]->pCSRV
		//);

		////Albedo Pass
		//RenderFrames[FrameIndex].pGCL->BeginRenderPass(1, &renderPassRenderTargetDesc, &renderPassDepthStencilDesc, D3D12_RENDER_PASS_FLAG_NONE);
		//// Record Commands

		//RenderFrames[FrameIndex].pGCL->SetPipelineState1(RenderingPipelines->GetStateObject("Albedo"));


		//RenderFrames[FrameIndex].pGCL->EndRenderPass();
	}


	void D3D12RenderingApi::OnEndRender()
	{
		CD3DX12_RESOURCE_BARRIER rtBarriers[GBUFFER_SIZE] = {  };

		for (INT32 i = 0; i < GBUFFER_SIZE; ++i)
		{
			rtBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[i]->pResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		RenderFrames[FrameIndex].pGCL->ResourceBarrier(GBUFFER_SIZE, &rtBarriers[0]);
	}


	void D3D12RenderingApi::DrawSceneStaticGeometry(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems)
	{
		const auto dx12Pso = dynamic_cast<D3D12PipelineStateObject*>(pso);
		RenderFrames[FrameIndex].pGCL->SetPipelineState(dx12Pso->GetPipelineState());
		
		// For each render item...
		for (auto& renderItem : renderItems)
		{

			const auto renderItemDerived = dynamic_cast<D3D12RenderItem*>(renderItem);
			const auto d3d12VertexBuffer = dynamic_cast<D3D12VertexBuffer*>(renderItem->Geometry->VertexBuffer.get());
			const auto d3d12IndexBuffer = dynamic_cast<D3D12IndexBuffer*>(renderItem->Geometry->IndexBuffer.get());

			RenderFrames[FrameIndex].pGCL->IASetVertexBuffers(0, 1, &d3d12VertexBuffer->GetVertexBufferView());
			RenderFrames[FrameIndex].pGCL->IASetIndexBuffer(&d3d12IndexBuffer->GetIndexBufferView());
			RenderFrames[FrameIndex].pGCL->IASetPrimitiveTopology(renderItemDerived->PrimitiveType);

			const UINT objConstBufferByteSize = D3D12BufferFactory::CalculateBufferByteSize(sizeof(ObjectConstant));
			const UINT matConstBufferByteSize = D3D12BufferFactory::CalculateBufferByteSize(sizeof(MaterialConstants));

			ID3D12Resource* objectConstantBuffer	= RenderFrames[FrameIndex].ConstantBuffer.Resource();

			const D3D12_GPU_VIRTUAL_ADDRESS objConstBufferAddress = objectConstantBuffer->GetGPUVirtualAddress() + renderItem->ObjectConstantBufferIndex * objConstBufferByteSize;

			RenderFrames[FrameIndex].pGCL->SetGraphicsRootConstantBufferView(0, objConstBufferAddress);
			RenderFrames[FrameIndex].pGCL->DrawIndexedInstanced(renderItem->Geometry->IndexBuffer->GetCount(), 1, renderItem->StartIndexLocation, renderItem->BaseVertexLocation, 0);

		}
	}

	GraphicsContext* D3D12RenderingApi::GetGraphicsContext() const
	{
		return Context; 
	}

	FrameBuffer* D3D12RenderingApi::GetFrameBuffer() const
	{
		return FrameBuffer.get();
	}

	const RenderTarget* D3D12RenderingApi::GetSceneAlbedoTexture() const
	{
		return RenderTargets[static_cast<UINT32>(RenderLayer::Albedo)].get();
	}

	const RenderTarget* D3D12RenderingApi::GetSceneNormalTexture() const
	{
		return RenderTargets[static_cast<UINT32>(RenderLayer::Normals)].get();
	}

	const RenderTarget* D3D12RenderingApi::GetSceneAmbientOcclusionTexture() const
	{
		return RenderTargets[static_cast<UINT32>(RenderLayer::AmbientOcclusion)].get();
	}

	const RenderTarget* D3D12RenderingApi::GetSceneDepthTexture() const
	{
		return RenderTargets[static_cast<UINT32>(RenderLayer::Depth)].get();
	}

	void D3D12RenderingApi::Flush()
	{
		HRESULT hr{ S_OK };
		hr = RenderFrames[FrameIndex].pGCL->Close();
		THROW_ON_FAILURE(hr);
		ID3D12CommandList* cmdList[] = { RenderFrames[FrameIndex].pGCL.Get() };
		Context->pQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
		RenderFrames[FrameIndex].Fence = ++Context->SyncCounter;
		hr = Context->pQueue->Signal(Context->pFence.Get(), RenderFrames[FrameIndex].Fence);
		THROW_ON_FAILURE(hr);
	}


}
