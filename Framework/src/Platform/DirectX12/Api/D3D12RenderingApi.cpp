#include "D3D12RenderingApi.h"
#include "Framework/Core/Log/Log.h"

#include "Platform/Directx12/Buffers/D3D12FrameBuffer.h"
#include "Platform/Directx12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/Pipeline/D3D12RenderPipeline.h"
#include "Platform/DirectX12/Resources/D3D12RenderFrame.h"
#include "Platform/DirectX12/Heap/D3D12HeapManager.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineResource.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Platform/DirectX12/Utilities/D3D12BufferFactory.h"
#include "Platform/DirectX12/Textures/D3D12RenderTarget.h"
#include "Platform/DirectX12/Textures/Loader/D3D12TextureLoader.h"
#include "Platform/DirectX12/D3D12/D3D12.h"

namespace Foundation::Graphics::D3D12
{


	D3D12RenderingApi::D3D12RenderingApi()
	{
	}

	D3D12RenderingApi::~D3D12RenderingApi()
	{
	}

	void D3D12RenderingApi::Clean()
	{
	}

	void D3D12RenderingApi::SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		
		CORE_TRACE("Buffer resize w: {0}, h: {1}", width, height);
		FrameBufferSpecifications fbSpecs = {};
		fbSpecs.Width = width;
		fbSpecs.Height = height;
		fbSpecs.OffsetX = x;
		fbSpecs.OffsetY = y;

		
		
	}

	void D3D12RenderingApi::PreInit()
	{
		auto* pCommandList = gD3D12Context->GetGraphicsCommandList();
		auto* pCommandAlloc = gD3D12Context->GetGraphicsCommandAllocator();

		HRESULT hr{ S_OK };
		hr = pCommandList->Reset(pCommandAlloc, nullptr);
		THROW_ON_FAILURE(hr);
	}

	void D3D12RenderingApi::PostInit()
	{
		auto* pCommandList = gD3D12Context->GetGraphicsCommandList();
		auto* pQueue = gD3D12Context->GetRenderingQueue();
		// Execute the initialization commands.
		HRESULT hr = pCommandList->Close();
		THROW_ON_FAILURE(hr);
		ID3D12CommandList* pList[] = { pCommandList };
		pQueue->ExecuteCommandLists(_countof(pList), pList);
		//Flush();
	}

	void D3D12RenderingApi::OnBeginRender()
	{
		// If everything checks out prepare recording instructions under the
		// current frame resource.
		auto* pQueue = gD3D12Context->GetRenderingQueue();
		CORE_ASSERT(pQueue, "Invalid command queue...");

		gD3D12Context->IncrementFrame();
		const auto frame = gD3D12Context->CurrentRenderFrame();

		CORE_ASSERT(frame, "Invalid frame..");
		THROW_ON_FAILURE(frame->pCmdAlloc->Reset());
		THROW_ON_FAILURE(frame->pGCL->Reset(frame->pCmdAlloc.Get(), nullptr));

	}

	void D3D12RenderingApi::OnEndRender()
	{
		const auto frame = gD3D12Context->CurrentRenderFrame();
		
		Flush();
	}

	void D3D12RenderingApi::Flush()
	{
		auto* pQueue = gD3D12Context->GetRenderingQueue();
		const auto frame = gD3D12Context->CurrentRenderFrame();


		HRESULT hr{ S_OK };
		hr = frame->pGCL->Close();
		THROW_ON_FAILURE(hr);
		ID3D12CommandList* cmdList[] = { frame->pGCL.Get() };
		pQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
		
		frame->Fence = gD3D12Context->GetSyncCount();
		hr = pQueue->Signal(gD3D12Context->GetFence(), frame->Fence);
		THROW_ON_FAILURE(hr);
	}

	void D3D12RenderingApi::Draw(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer)
	{
	}

	void D3D12RenderingApi::DrawIndexed(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer,
		IndexBuffer* indexBuffer)
	{
	}

	void D3D12RenderingApi::DrawInstanced(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer,
		UINT32 instanceCount)
	{
	}

	void D3D12RenderingApi::DrawIndexedInstanced(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer,
		IndexBuffer* indexBuffer, UINT32 instanceCount)
	{
	}
}
