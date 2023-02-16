#include "D3D12ComputeApi.h"

#include "D3D12Context.h"
#include "Framework/Core/Log/Log.h"

namespace Engine
{
	void D3D12ComputeApi::Init(GraphicsContext* context)
	{
		auto d3dContext = dynamic_cast<D3D12Context*>(context);

		Device = d3dContext->Device.Get();

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		const HRESULT queueResult = Device->CreateCommandQueue(
			&desc,
			IID_PPV_ARGS(ComputeQueue.GetAddressOf()));
		THROW_ON_FAILURE(queueResult);

		const HRESULT cmdAllocResult = Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
			IID_PPV_ARGS(CommandAllocator.GetAddressOf()
			));
		THROW_ON_FAILURE(cmdAllocResult);

		const HRESULT cmdListResult = Device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
			CommandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(ComputeCommandList.GetAddressOf()));
		THROW_ON_FAILURE(cmdListResult);

		const HRESULT closureResult = ComputeCommandList->Close();
		THROW_ON_FAILURE(closureResult);

	}

	void D3D12ComputeApi::Dispatch(const void* data, INT32 x, INT32 y, INT32 z)
	{
	}

	void D3D12ComputeApi::ResetComputeCommandList()
	{
	}

	void D3D12ComputeApi::ExecuteComputeCommandList()
	{
	}
}
