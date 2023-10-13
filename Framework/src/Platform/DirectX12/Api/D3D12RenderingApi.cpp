#include "D3D12RenderingApi.h"
#include "Framework/Core/Log/Log.h"

#include "Platform/DirectX12/D3D12/D3D12.h"
#include "Platform/DirectX12/Pipeline/D3D12RenderPipeline.h"
#include "Platform/Directx12/Buffers/D3D12Buffers.h"


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

	void D3D12RenderingApi::BeginRender()
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

	void D3D12RenderingApi::EndRender()
	{
		const auto frame = gD3D12Context->CurrentRenderFrame();

		Flush();
	}

	void D3D12RenderingApi::Flush()
	{
		auto* pQueue = gD3D12Context->GetRenderingQueue();
		const auto frame = gD3D12Context->CurrentRenderFrame();


		HRESULT hr{ S_OK };
		hr = frame->GetFrameGraphicsCommandList()->Close();
		THROW_ON_FAILURE(hr);
		ID3D12CommandList* cmdList[] = { frame->GetFrameGraphicsCommandList() };
		pQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

		auto& fence = frame->GetFence();
		fence = gD3D12Context->GetSyncCount();
		hr = pQueue->Signal(gD3D12Context->GetFence(), fence);
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
