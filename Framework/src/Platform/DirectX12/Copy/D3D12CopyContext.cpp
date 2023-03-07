#include "D3D12CopyContext.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Framework/Core/Log/Log.h"

namespace Engine
{

	ComPtr<ID3D12CommandAllocator> D3D12CopyContext::Allocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> D3D12CopyContext::CmdList = nullptr;
	ComPtr<ID3D12CommandQueue> D3D12CopyContext::Queue = nullptr;
	UINT64 D3D12CopyContext::FenceValue = 0;

	void D3D12CopyContext::Init(D3D12Context* context)
	{

		Context = dynamic_cast<D3D12Context*>(context);

		FenceValue = Context->GPU_TO_CPU_SYNC_COUNT;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.Priority = 0;
		desc.NodeMask = 0;

		const HRESULT queueResult = Context->Device->CreateCommandQueue(
			&desc,
			IID_PPV_ARGS(Queue.GetAddressOf()));
		THROW_ON_FAILURE(queueResult);

		const HRESULT cmdAllocResult = Context->Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
			IID_PPV_ARGS(Allocator.GetAddressOf()
			));
		THROW_ON_FAILURE(cmdAllocResult);

		const HRESULT cmdListResult = Context->Device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
			Allocator.Get(),
			nullptr,
			IID_PPV_ARGS(CmdList.GetAddressOf()));
		THROW_ON_FAILURE(cmdListResult);

		const HRESULT closeResult = CmdList->Close();
		THROW_ON_FAILURE(closeResult);

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

		CmdList->CopyTextureRegion(&copyDst, 0, 0, 0, &copySrc, nullptr);

	}

	void D3D12CopyContext::CopyBuffer(ID3D12Resource* src, ID3D12Resource* dst, INT32 x, INT32 y, INT32 z)
	{
	}
}
