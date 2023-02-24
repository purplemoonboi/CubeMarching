#include "D3D12ComputeApi.h"

#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Framework/Core/Log/Log.h"

namespace Engine
{
	void D3D12ComputeApi::Init(GraphicsContext* context)
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
			IID_PPV_ARGS(CommandAllocator.GetAddressOf()
			));
		THROW_ON_FAILURE(cmdAllocResult);

		const HRESULT cmdListResult = Context->Device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
			CommandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(CommandList.GetAddressOf()));
		THROW_ON_FAILURE(cmdListResult);

		const HRESULT closeResult = CommandList->Close();
		THROW_ON_FAILURE(closeResult);

	}


	void D3D12ComputeApi::ResetComputeCommandList(PipelineStateObject* state)
	{
		const HRESULT allocResult = CommandAllocator->Reset();
		THROW_ON_FAILURE(allocResult);

		auto const d3d12PipelineState = dynamic_cast<D3D12PipelineStateObject*>(state);
		const HRESULT closeResult = CommandList->Reset(CommandAllocator.Get(),
			d3d12PipelineState->GetPipelineState());
		THROW_ON_FAILURE(closeResult);
	}

	void D3D12ComputeApi::ExecuteComputeCommandList()
	{
		
		const HRESULT closeResult = CommandList->Close();
		THROW_ON_FAILURE(closeResult);

		ID3D12CommandList* cmdList[] = { CommandList.Get() };
		Queue->ExecuteCommandLists(_countof(cmdList), cmdList);

		FenceValue = ++Context->GPU_TO_CPU_SYNC_COUNT;
		const HRESULT signalResult = Queue->Signal(Context->Fence.Get(), FenceValue);
		THROW_ON_FAILURE(signalResult);
	}

	void D3D12ComputeApi::FlushComputeQueue()
	{
		const HRESULT closeResult = CommandList->Close();
		THROW_ON_FAILURE(closeResult);

		ID3D12CommandList* cmdList[] = { CommandList.Get() };
		Queue->ExecuteCommandLists(_countof(cmdList), cmdList);

		FenceValue = ++Context->GPU_TO_CPU_SYNC_COUNT;
		const HRESULT signalResult = Queue->Signal(Context->Fence.Get(), FenceValue);
		THROW_ON_FAILURE(signalResult);

		auto const completedValue = Context->Fence->GetCompletedValue();
		if (completedValue < FenceValue)
		{
			const HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			THROW_ON_FAILURE(Context->Fence->SetEventOnCompletion(FenceValue, eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}

	}
}