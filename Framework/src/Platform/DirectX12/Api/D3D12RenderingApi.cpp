#include "D3D12RenderingApi.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/Directx12/Buffers/D3D12FrameBuffer.h"
#include "Platform/Directx12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/RenderItems/D3D12RenderItem.h"
#include "Platform/DirectX12/Pipeline/D3D12RenderPipeline.h"
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


	D3D12RenderingApi::D3D12RenderingApi()
		:
		Context(nullptr)
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
			//FrameBuffer->OnResizeFrameBuffer(fbSpecs);
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
		//Flush();
	}

	void D3D12RenderingApi::OnBeginRender()
	{
		// If everything checks out prepare recording instructions under the
		// current frame resource.
		CORE_ASSERT(Context->pQueue, "Invalid command queue...");

		IncrementFrame();
		const auto frame = CurrentRenderFrame();

		CORE_ASSERT(frame, "Invalid frame..");
		THROW_ON_FAILURE(frame->pCmdAlloc->Reset());
		THROW_ON_FAILURE(frame->pGCL->Reset(frame->pCmdAlloc.Get(), nullptr));


		
		
	}


	void D3D12RenderingApi::OnEndRender()
	{
		const auto frame = CurrentRenderFrame();
		
		Flush();
	}

	void D3D12RenderingApi::DrawSceneStaticGeometry(RenderPipeline* pso, const std::vector<RenderItem*>& renderItems)
	{
		const auto frame = CurrentRenderFrame();

		const auto dx12Pso = dynamic_cast<D3D12RenderPipeline*>(pso);
		frame->pGCL->SetPipelineState(dx12Pso->GetPipelineState());
		
		// For each render item...
		for (auto& renderItem : renderItems)
		{

			const auto renderItemDerived	=	dynamic_cast<D3D12RenderItem*>(renderItem);
			const auto d3d12VertexBuffer	=	dynamic_cast<D3D12VertexBuffer*>(renderItem->Geometry->VertexBuffer.get());
			const auto d3d12IndexBuffer		=	dynamic_cast<D3D12IndexBuffer*>(renderItem->Geometry->IndexBuffer.get());

			frame->pGCL->IASetVertexBuffers(0, 1, &d3d12VertexBuffer->GetView());
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


	void D3D12RenderingApi::Flush()
	{
		const auto frame = CurrentRenderFrame();

		HRESULT hr{ S_OK };
		hr = frame->pGCL->Close();
		THROW_ON_FAILURE(hr);
		ID3D12CommandList* cmdList[] = { frame->pGCL.Get() };
		Context->pQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
		Context->IncrementSync();
		frame->Fence = Context->GetSyncCount();
		hr = Context->pQueue->Signal(GetFence(), frame->Fence);
		THROW_ON_FAILURE(hr);
	}


}
