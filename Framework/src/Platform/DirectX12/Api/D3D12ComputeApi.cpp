
#include <Platform/DirectX12/Compute/D3D12ComputeApi.h>

#include "D3D12Context.h"
#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"

namespace Engine
{
	void D3D12ComputeApi::Init(GraphicsContext* context)
	{
		Context = dynamic_cast<D3D12Context*>(context);


		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

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

		const HRESULT closureResult = CommandList->Close();
		THROW_ON_FAILURE(closureResult);

	}

	void D3D12ComputeApi::ResetComputeCommandList(PipelineStateObject* state)
	{
		auto d3d12Pso = dynamic_cast<D3D12PipelineStateObject*>(state);
		const HRESULT cmdAllocResult = CommandAllocator->Reset();
		THROW_ON_FAILURE(cmdAllocResult);
		const HRESULT cmdListResult = CommandList->Reset(CommandAllocator.Get(),
			d3d12Pso->GetPipelineState());
		THROW_ON_FAILURE(cmdListResult);


	}

	void D3D12ComputeApi::ExecuteComputeCommandList()
	{
		const HRESULT closeResult = CommandList->Close();
		THROW_ON_FAILURE(closeResult);
		ID3D12CommandList* cmdList[] = { CommandList.Get() };
		Queue->ExecuteCommandLists(_countof(cmdList), cmdList);

		FenceValue = ++Context->GPU_TO_CPU_SYNC_COUNT;
		Queue->Signal(Context->Fence.Get(), FenceValue);
	}
}
