#include "D3D12MemoryManager.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Api/D3D12Context.h"


namespace Engine
{
	D3D12MemoryManager::~D3D12MemoryManager()
	{
	}

	void D3D12MemoryManager::Allocate(UINT64 ptr, UINT64 memSize)
	{
	}

	void D3D12MemoryManager::Deallocate(UINT64 ptr, UINT64 memSize)
	{
	}

	HRESULT D3D12MemoryManager::InitialiseSrvUavHeap(D3D12Context* context, UINT64 size)
	{
		HRESULT allocResult = S_FALSE;
		if(!IsInitialised)
		{
			/**
			 * create our SRV heap
			 */
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			srvHeapDesc.NumDescriptors = size + 1U;// +1 for ImGui
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			const HRESULT heapResult = context->Device->CreateDescriptorHeap(&srvHeapDesc,
				IID_PPV_ARGS(&SrvUavHeap));
			THROW_ON_FAILURE(heapResult);

			const HRESULT deviceRemovedReasonSrvHeap = context->Device->GetDeviceRemovedReason();
			THROW_ON_FAILURE(deviceRemovedReasonSrvHeap);

			SrvUavDescriptorSize = context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			ResourceHandles.CpuCurrentHandle = SrvUavHeap->GetCPUDescriptorHandleForHeapStart();
			ResourceHandles.GpuCurrentHandle = SrvUavHeap->GetGPUDescriptorHandleForHeapStart();

			ImGuiHandleOffset = ((size + 1U) - 1U);
			MaxHandleOffset = ImGuiHandleOffset;
			ImGuiHandles.CpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetCPUDescriptorHandleForHeapStart(), ImGuiHandleOffset, SrvUavDescriptorSize);
			ImGuiHandles.GpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetGPUDescriptorHandleForHeapStart(), ImGuiHandleOffset, SrvUavDescriptorSize);

			IsInitialised = true;
			allocResult = heapResult;
		}

		return allocResult;
	}

	D3D12MemoryManager::Handles D3D12MemoryManager::GetResourceHandle(INT32 off)
	{
	
		const auto offset = HandleOffset;
		HandleOffset += off;

		if(offset == ImGuiHandleOffset)
		{
			return
			{
				CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetGPUDescriptorHandleForHeapStart(), 0, SrvUavDescriptorSize),
				CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetCPUDescriptorHandleForHeapStart(),  0, SrvUavDescriptorSize),
				false
			};
		}

		return {
			 CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetGPUDescriptorHandleForHeapStart(), offset, SrvUavDescriptorSize),
			 CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetCPUDescriptorHandleForHeapStart(),  offset, SrvUavDescriptorSize)
		};
	}
}

