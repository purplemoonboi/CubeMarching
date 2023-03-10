#include "D3D12ComputeApi.h"

#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Framework/Core/Log/Log.h"

namespace Engine
{
	void D3D12ComputeApi::Init(GraphicsContext* context)
	{
		Context = dynamic_cast<D3D12Context*>(context);

		FenceValue = 0;

		const HRESULT fenceResult = Context->Device->CreateFence
		(
			0,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&Fence)
		);
		THROW_ON_FAILURE(fenceResult);

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

		if(state != nullptr)
		{
			auto const d3d12PipelineState = dynamic_cast<D3D12PipelineStateObject*>(state);
			const HRESULT resetResult = CommandList->Reset(CommandAllocator.Get(),
				d3d12PipelineState->GetPipelineState());
			THROW_ON_FAILURE(resetResult);
		}
		else
		{
			const HRESULT resetResult = CommandList->Reset(CommandAllocator.Get(),
				nullptr);
			THROW_ON_FAILURE(resetResult);
		}
	}

	void D3D12ComputeApi::ExecuteComputeCommandList(UINT64* voxelWorldSyncValue)
	{
		
		const HRESULT closeResult = CommandList->Close();
		THROW_ON_FAILURE(closeResult);

		ID3D12CommandList* cmdList[] = { CommandList.Get() };
		Queue->ExecuteCommandLists(_countof(cmdList), cmdList);

		*voxelWorldSyncValue = ++FenceValue;

		const HRESULT signalResult = Queue->Signal(Fence.Get(), *voxelWorldSyncValue);
		THROW_ON_FAILURE(signalResult);
	}

	void D3D12ComputeApi::FlushComputeQueue(UINT64* voxelWorldSyncValue)
	{
		const HRESULT closeResult = CommandList->Close();
		THROW_ON_FAILURE(closeResult);

		ID3D12CommandList* cmdList[] = { CommandList.Get() };
		Queue->ExecuteCommandLists(_countof(cmdList), cmdList);

		//FenceValue = ++FenceValue;
	
		*voxelWorldSyncValue = ++FenceValue;
		const HRESULT signalResult = Queue->Signal(Fence.Get(), *voxelWorldSyncValue);
		THROW_ON_FAILURE(signalResult);

		auto const completedValue = Fence->GetCompletedValue();

		if (completedValue < *voxelWorldSyncValue)
		{
			const HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			THROW_ON_FAILURE(Fence->SetEventOnCompletion(*voxelWorldSyncValue, eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}

	}
	void D3D12ComputeApi::Wait(UINT64* voxelWorldSyncValue)
	{
		// Has the GPU finished processing the commands of the current frame resource?
		// If not, wait until the GPU has completed commands up to this fence point.
		const UINT64 a = *voxelWorldSyncValue;
		const UINT64 b = Context->Fence->GetCompletedValue();
		if (a != 0 && b < a)
		{
			const HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			const HRESULT eventCompletion = Context->Fence->SetEventOnCompletion(*voxelWorldSyncValue, eventHandle);
			THROW_ON_FAILURE(eventCompletion);
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}
}
