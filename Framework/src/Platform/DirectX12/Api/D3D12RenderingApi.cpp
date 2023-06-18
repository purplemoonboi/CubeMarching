#include "D3D12RenderingApi.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/Directx12/Buffers/D3D12FrameBuffer.h"
#include "Platform/Directx12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/RenderItems/D3D12RenderItem.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Resources/D3D12RenderFrame.h"
#include "Platform/DirectX12/Heap/D3D12HeapManager.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineResource.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Platform/DirectX12/Utilities/D3D12BufferFactory.h"
#include "Platform/DirectX12/Textures/D3D12RenderTarget.h"
#include "Platform/DirectX12/Textures/Loader/D3D12TextureLoader.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
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

	void D3D12RenderingApi::OnBeginRender()
	{
		// If everything checks out prepare recording instructions under the
		// current frame resource.
		CORE_ASSERT(Context->pQueue, "Invalid command queue...");

		const auto frame = IncrementFrame();

		CORE_ASSERT(frame, "Invalid frame..");
		THROW_ON_FAILURE(frame->pCmdAlloc->Reset());
		THROW_ON_FAILURE(frame->pGCL->Reset(frame->pCmdAlloc.Get(), nullptr));


		CD3DX12_RESOURCE_BARRIER rtBarriers[GBUFFER_SIZE] = {  };

		for(INT32 i = 0; i < GBUFFER_SIZE; ++i)
		{
			rtBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[i]->pResource.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, 
				D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		// Bind core render root signature.
		// Bind the resource descriptor heap and keep it live until the end of the passes.
		frame->pGCL->ResourceBarrier(GBUFFER_SIZE, &rtBarriers[0]);
		frame->pGCL->SetGraphicsRootSignature(pRootSignature.Get());

		ID3D12DescriptorHeap* descriptorHeaps[] = { GetSRVHeap()->GetHeap() };
		frame->pGCL->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		const D3D12_GPU_VIRTUAL_ADDRESS passBufferAddress = frame->PassBuffer.Resource()->GetGPUVirtualAddress();
		frame->pGCL->SetGraphicsRootConstantBufferView(2, passBufferAddress);

		// We can bind all textures in the scene - we declared 'n' amount of descriptors in the root signature.
		frame->pGCL->SetGraphicsRootDescriptorTable(3, GetSRVHeap()->BeginGPU());
		
	}

	void D3D12RenderingApi::BindPasses()
	{
		/*
		frame->pGCL->RSSetViewports(1, &RenderTargets[(INT8)RenderLayer::Albedo].Viewport);
		frame->pGCL->RSSetScissorRects(1, &RenderTargets[(INT8)RenderLayer::Albedo].Rect);

		// Specify the buffers we are going to render to.
		frame->pGCL->OMSetRenderTargets(1, &RenderTargets[(INT8)RenderLayer::Albedo].pRTV,
			true, &FrameBuffer.GetDepthStencilViewCpu());

		// Clear the back buffer and depth buffer.
		frame->pGCL->ClearRenderTargetView(RenderTargets[(INT8)RenderLayer::Albedo].pRTV, DirectX::Colors::LightBlue, 0, nullptr);
		frame->pGCL->ClearDepthStencilView(FrameBuffer.GetDepthStencilViewCpu(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		//frame->pGCL->SetPipelineState();
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
		//frame->pGCL->OMSetRenderTargets(1, &RenderTargets[static_cast<UINT32>(RenderLayer::Albedo)]->pRTV, 
		//	FALSE,
		//	&RenderTargets[static_cast<UINT32>(RenderLayer::Albedo)]->pCSRV
		//);

		////Albedo Pass
		//frame->pGCL->BeginRenderPass(1, &renderPassRenderTargetDesc, &renderPassDepthStencilDesc, D3D12_RENDER_PASS_FLAG_NONE);
		//// Record Commands

		//frame->pGCL->SetPipelineState1(RenderingPipelines->GetStateObject("Albedo"));


		//frame->pGCL->EndRenderPass();
	}


	void D3D12RenderingApi::OnEndRender()
	{
		const auto frame = CurrentRenderFrame();
		CD3DX12_RESOURCE_BARRIER rtBarriers[GBUFFER_SIZE] = {  };

		for (INT32 i = 0; i < GBUFFER_SIZE; ++i)
		{
			rtBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[i]->pResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		frame->pGCL->ResourceBarrier(GBUFFER_SIZE, &rtBarriers[0]);
	}


	void D3D12RenderingApi::DrawSceneStaticGeometry(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems)
	{
		const auto frame = CurrentRenderFrame();

		const auto dx12Pso = dynamic_cast<D3D12PipelineStateObject*>(pso);
		frame->pGCL->SetPipelineState(dx12Pso->GetPipelineState());
		
		// For each render item...
		for (auto& renderItem : renderItems)
		{

			const auto renderItemDerived = dynamic_cast<D3D12RenderItem*>(renderItem);
			const auto d3d12VertexBuffer = dynamic_cast<D3D12VertexBuffer*>(renderItem->Geometry->VertexBuffer.get());
			const auto d3d12IndexBuffer = dynamic_cast<D3D12IndexBuffer*>(renderItem->Geometry->IndexBuffer.get());

			frame->pGCL->IASetVertexBuffers(0, 1, &d3d12VertexBuffer->GetVertexBufferView());
			frame->pGCL->IASetIndexBuffer(&d3d12IndexBuffer->GetIndexBufferView());
			frame->pGCL->IASetPrimitiveTopology(renderItemDerived->PrimitiveType);

			const UINT objConstBufferByteSize = D3D12BufferFactory::CalculateBufferByteSize(sizeof(ObjectConstant));
			const UINT matConstBufferByteSize = D3D12BufferFactory::CalculateBufferByteSize(sizeof(MaterialConstants));

			ID3D12Resource* objectConstantBuffer	= frame->ConstantBuffer.Resource();

			const D3D12_GPU_VIRTUAL_ADDRESS objConstBufferAddress = objectConstantBuffer->GetGPUVirtualAddress() + renderItem->ObjectConstantBufferIndex * objConstBufferByteSize;

			frame->pGCL->SetGraphicsRootConstantBufferView(0, objConstBufferAddress);
			frame->pGCL->DrawIndexedInstanced(renderItem->Geometry->IndexBuffer->GetCount(), 1, renderItem->StartIndexLocation, renderItem->BaseVertexLocation, 0);

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
		const auto frame = CurrentRenderFrame();

		HRESULT hr{ S_OK };
		hr = frame->pGCL->Close();
		THROW_ON_FAILURE(hr);
		ID3D12CommandList* cmdList[] = { frame->pGCL.Get() };
		Context->pQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
		frame->Fence = ++Context->SyncCounter;
		hr = Context->pQueue->Signal(Context->pFence.Get(), frame->Fence);
		THROW_ON_FAILURE(hr);
	}


}
