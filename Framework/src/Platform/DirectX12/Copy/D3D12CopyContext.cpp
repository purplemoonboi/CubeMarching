#include "D3D12CopyContext.h"
#include "Framework/Core/Log/Log.h"

#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Foundation
{

	ComPtr<ID3D12CommandAllocator> D3D12CopyContext::Allocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> D3D12CopyContext::CmdList = nullptr;
	ComPtr<ID3D12CommandQueue> D3D12CopyContext::Queue = nullptr;
	D3D12Context* D3D12CopyContext::Context = nullptr;

	UINT64 D3D12CopyContext::FenceValue = 0;

	void D3D12CopyContext::Init(D3D12Context* context)
	{

		Context = dynamic_cast<D3D12Context*>(context);

		FenceValue = Context->SyncCounter;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.Priority = 0;
		desc.NodeMask = 0;

		const HRESULT queueResult = Context->pDevice->CreateCommandQueue(
			&desc,
			IID_PPV_ARGS(Queue.GetAddressOf()));
		THROW_ON_FAILURE(queueResult);

		const HRESULT cmdAllocResult = Context->pDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_COPY,
			IID_PPV_ARGS(Allocator.GetAddressOf()
			));
		THROW_ON_FAILURE(cmdAllocResult);

		const HRESULT cmdListResult = Context->pDevice->CreateCommandList(
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
		Allocator->SetName(L"Copy ResourceAlloc");
		

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

		const HRESULT signal = Queue->Signal(Context->pFence.Get(), FenceValue);
		THROW_ON_FAILURE(signal);
	}

	void D3D12CopyContext::CopyTexture(ID3D12Resource* src, ID3D12Resource* dst, INT32 x, INT32 y, INT32 z)
	{
		ResetCopyList(nullptr);

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

	void D3D12CopyContext::UpdateVertexBuffer(D3D12VertexBuffer* vertexBuffer)
	{
		// Give a desc of the data we want to copy
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = vertexBuffer->Blob.Get();
		subResourceData.RowPitch = vertexBuffer->GetCount() * sizeof(Vertex);
		subResourceData.SlicePitch = subResourceData.RowPitch;

		// Schedule to copy the data to the default buffer resource.
		// Make instruction to copy CPU buffer into intermediate upload heap
		// buffer.
		CmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				vertexBuffer->DefaultBuffer.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_GENERIC_READ
			)
		);

		// Copy the data into the upload heap
		UpdateSubresources<1>
			(
				CmdList.Get(),
				vertexBuffer->DefaultBuffer.Get(),
				vertexBuffer->UploadBuffer.Get(),
				0,
				0,
				1,
				&subResourceData
				);

		// Add the instruction to transition back to read 
		CmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				vertexBuffer->DefaultBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_GENERIC_READ
			)
		);

	}
	
}
