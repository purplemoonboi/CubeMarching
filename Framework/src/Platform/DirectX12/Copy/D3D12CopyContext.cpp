#include "D3D12CopyContext.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Framework/Core/Log/Log.h"

namespace Engine
{

	ComPtr<ID3D12CommandAllocator> D3D12CopyContext::Allocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> D3D12CopyContext::CmdList = nullptr;
	ComPtr<ID3D12CommandQueue> D3D12CopyContext::Queue = nullptr;
	D3D12Context* D3D12CopyContext::Context = nullptr;

	UINT64 D3D12CopyContext::FenceValue = 0;

	void D3D12CopyContext::Init(D3D12Context* context)
	{

		Context = dynamic_cast<D3D12Context*>(context);

		FenceValue = Context->GPU_TO_CPU_SYNC_COUNT;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.Priority = 0;
		desc.NodeMask = 0;

		const HRESULT queueResult = Context->Device->CreateCommandQueue(
			&desc,
			IID_PPV_ARGS(Queue.GetAddressOf()));
		THROW_ON_FAILURE(queueResult);

		const HRESULT cmdAllocResult = Context->Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_COPY,
			IID_PPV_ARGS(Allocator.GetAddressOf()
			));
		THROW_ON_FAILURE(cmdAllocResult);

		const HRESULT cmdListResult = Context->Device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_COPY,
			Allocator.Get(),
			nullptr,
			IID_PPV_ARGS(CmdList.GetAddressOf()));
		THROW_ON_FAILURE(cmdListResult);

		const HRESULT closeResult = CmdList->Close();
		THROW_ON_FAILURE(closeResult);

		CmdList->SetName(L"Copy List");
		Queue->SetName(L"Copy Queue");
		Allocator->SetName(L"Copy Allocator");
		CmdList->Reset(Allocator.Get(), nullptr);

	}

	void D3D12CopyContext::ResetCopyList(ID3D12PipelineState* state)
	{

		const HRESULT allocResult = Allocator->Reset();
		THROW_ON_FAILURE(allocResult);

		const HRESULT cmdResult = CmdList->Reset(Allocator.Get(), state);
		THROW_ON_FAILURE(cmdResult);

	}

	void D3D12CopyContext::ExecuteCopyList()
	{
		const HRESULT closeResult = CmdList->Close();
		THROW_ON_FAILURE(closeResult);

		ID3D12CommandList* cmdList[] = { CmdList.Get() };
		Queue->ExecuteCommandLists(_countof(cmdList), cmdList);

		FenceValue = ++FenceValue;

		const HRESULT signal = Queue->Signal(Context->Fence.Get(), FenceValue);
		THROW_ON_FAILURE(signal);
	}

	void D3D12CopyContext::CopyTexture(ID3D12Resource* src, ID3D12Resource* dst, INT32 x, INT32 y, INT32 z)
	{
		D3D12_TEXTURE_COPY_LOCATION copySrc = {};
		copySrc.pResource = src;
		copySrc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		copySrc.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION copyDst = {};
		copyDst.pResource = dst;
		copyDst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		copyDst.SubresourceIndex = 0;

		CmdList->ResourceBarrier(1, 
			&CD3DX12_RESOURCE_BARRIER::Transition(src, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE)
		);

		CmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(dst, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST)
		);

		CmdList->CopyTextureRegion(&copyDst, 0, 0, 0, &copySrc, nullptr);

		CmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(src, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PRESENT)
		);

		CmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(dst, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON)
		);

		ExecuteCopyList();
	}

	void D3D12CopyContext::CopyBuffer(ID3D12Resource* src, UINT64 srcOffset, ID3D12Resource* dst, UINT64 dstOffset, UINT64 sizeInBytes)
	{

		CmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(src, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE)
		);

		CmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(dst, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST)
		);

		CmdList->CopyBufferRegion(dst, dstOffset, src, srcOffset, sizeInBytes);

		CmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(src, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON)
		);

		CmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(dst, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON)
		);

		ExecuteCopyList();
	}
	
}
