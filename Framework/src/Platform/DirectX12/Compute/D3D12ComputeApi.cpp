#include "D3D12ComputeApi.h"
#include "Framework/Core/Log/Log.h"

#include "Platform/DirectX12/Pipeline/D3D12RenderPipeline.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Foundation::Graphics::D3D12
{

	D3D12ComputeFrameResource::D3D12ComputeFrameResource(ID3D12Device* device)
	{

		/**
		 * Create a command allocator for this resource
		 */
		const HRESULT cmdAllocResult = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE,
			IID_PPV_ARGS(CommandAllocator.GetAddressOf()));
		THROW_ON_FAILURE(cmdAllocResult);


		const HRESULT hr=CommandAllocator->Reset();
		THROW_ON_FAILURE(hr);

		
	}

	D3D12ComputeFrameResource::~D3D12ComputeFrameResource()
	{
	}


	void D3D12ComputeApi::Init(GraphicsContext* context)
	{
		HRESULT hr{ S_OK };

		FenceValue = 0;

		hr = pDevice->CreateFence
		(
			0,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&Fence)
		);
		THROW_ON_FAILURE(hr);

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.Priority = 0;
		desc.NodeMask = 0;

		hr = pDevice->CreateCommandQueue(
			&desc,
			IID_PPV_ARGS(Queue.GetAddressOf()));
		THROW_ON_FAILURE(hr);

		hr = pDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
			IID_PPV_ARGS(CommandAllocator.GetAddressOf()
			));
		THROW_ON_FAILURE(hr);

		hr = pDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
			CommandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(CommandList.GetAddressOf()));
		THROW_ON_FAILURE(hr);

		for(INT32 i = 0; i < NUMBER_OF_CS_FRAMES_IN_FLIGHT;++i)
		{
			CsFrameResources[i] = CreateScope<D3D12ComputeFrameResource>(pDevice.Get());
		}

		hr = CommandList->Close();
		THROW_ON_FAILURE(hr);


	}


	void D3D12ComputeApi::ResetComputeCommandList(RenderPipeline* state)
	{

		
		
		CurrentFrameResourceIndex = (CurrentFrameResourceIndex + 1) % NUMBER_OF_CS_FRAMES_IN_FLIGHT;
		CurrentCSFrameResource = CsFrameResources[CurrentFrameResourceIndex].get();
		CORE_ASSERT(CurrentCSFrameResource, "No valid frame resource!");

		// Has the GPU finished processing the commands of the current frame resource
		// If not, wait until the GPU has completed commands up to this fence point.
		if (Fence->GetCompletedValue() < CurrentCSFrameResource->Fence)
		{
			const HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			const HRESULT eventCompletion = Context->pFence->SetEventOnCompletion(CurrentCSFrameResource->Fence, eventHandle);
			THROW_ON_FAILURE(eventCompletion);
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}

		const HRESULT hr = CurrentCSFrameResource->CommandAllocator->Reset();
		THROW_ON_FAILURE(hr);

		if(state != nullptr)
		{
			auto const d3d12PipelineState = dynamic_cast<D3D12RenderPipeline*>(state);
			const HRESULT resetResult = CommandList->Reset(CurrentCSFrameResource->CommandAllocator.Get(),
				d3d12PipelineState->GetPipelineState());
			THROW_ON_FAILURE(resetResult);
		}
		else
		{
			const HRESULT resetResult = CommandList->Reset(CurrentCSFrameResource->CommandAllocator.Get(),
				nullptr);
			THROW_ON_FAILURE(resetResult);
		}
	}

	void D3D12ComputeApi::ExecuteComputeCommandList(UINT64* fence)
	{
		
		const HRESULT closeResult = CommandList->Close();
		THROW_ON_FAILURE(closeResult);

		ID3D12CommandList* cmdList[] = { CommandList.Get() };
		Queue->ExecuteCommandLists(_countof(cmdList), cmdList);

		*fence = ++CurrentCSFrameResource->Fence;

		const HRESULT signalResult = Queue->Signal(Fence.Get(), *fence);
		THROW_ON_FAILURE(signalResult);
	}

	void D3D12ComputeApi::FlushComputeQueue(UINT64* voxelWorldSyncValue)
	{


		const HRESULT closeResult = CommandList->Close();
		THROW_ON_FAILURE(closeResult);

		ID3D12CommandList* cmdList[] = { CommandList.Get() };
		Queue->ExecuteCommandLists(_countof(cmdList), cmdList);

	
		*voxelWorldSyncValue = ++CurrentCSFrameResource->Fence;
		const HRESULT signalResult = Queue->Signal(Fence.Get(), CurrentCSFrameResource->Fence);
		THROW_ON_FAILURE(signalResult);

		auto const completedValue = Fence->GetCompletedValue();

		if (completedValue < CurrentCSFrameResource->Fence)
		{
			const HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			THROW_ON_FAILURE(Fence->SetEventOnCompletion(CurrentCSFrameResource->Fence, eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}

	}
	void D3D12ComputeApi::Wait(UINT64* voxelWorldSyncValue)
	{
		
	}

	void D3D12ComputeApi::GlobalSignal(UINT64* gpuSync)
	{
		*gpuSync = ++CurrentCSFrameResource->Fence;
		const HRESULT signalResult = Queue->Signal(Fence.Get(), CurrentCSFrameResource->Fence);
		THROW_ON_FAILURE(signalResult);
		

	}
}
