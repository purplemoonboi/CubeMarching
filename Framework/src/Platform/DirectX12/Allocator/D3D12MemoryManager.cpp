#include "D3D12MemoryManager.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Api/D3D12Context.h"


namespace Engine
{
	D3D12MemoryManager::D3D12MemoryManager(ID3D12Device* device)
	{
		CbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	D3D12MemoryManager::~D3D12MemoryManager()
	{
	}

	void D3D12MemoryManager::Allocate(UINT64 size, UINT64 ptr)
	{
	}

	void D3D12MemoryManager::Deallocate(UINT64 size, UINT64 ptr)
	{

	}

	HRESULT D3D12MemoryManager::InitialiseCbvHeap(ID3D12Device* device, UINT64 frameResourceCount, UINT64 objectCount)
	{

		const UINT objCount = objectCount;
		// Need a CBV descriptor for each object for each frame resource,
		// +1 for the perPass CBV for each frame resource.
		const UINT numDescriptors = (objCount + 1) * frameResourceCount;

		// Save an offset to the start of the *pass* CBVs.  These are the last N descriptors.
		PassBufferOffsetSize = objCount * frameResourceCount;

		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = numDescriptors;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;

		THROW_ON_FAILURE
		(
			device->CreateDescriptorHeap
			(
				&cbvHeapDesc,
				IID_PPV_ARGS(&CbvHeap)
			)
		);



	}

	
	HRESULT D3D12MemoryManager::InitialiseSrvUavHeap(ID3D12Device* device, UINT64 size)
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
			const HRESULT heapResult = device->CreateDescriptorHeap(&srvHeapDesc,
				IID_PPV_ARGS(&SrvUavHeap));
			THROW_ON_FAILURE(heapResult);

			const HRESULT deviceRemovedReasonSrvHeap = device->GetDeviceRemovedReason();
			THROW_ON_FAILURE(deviceRemovedReasonSrvHeap);


			ResourceHandles.CpuCurrentHandle = SrvUavHeap->GetCPUDescriptorHandleForHeapStart();
			ResourceHandles.GpuCurrentHandle = SrvUavHeap->GetGPUDescriptorHandleForHeapStart();

			ImGuiHandleOffset = ((size + 1U) - 1U);
			MaxHandleOffset = ImGuiHandleOffset;
			ImGuiHandles.CpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetCPUDescriptorHandleForHeapStart(), ImGuiHandleOffset, CbvDescriptorSize);
			ImGuiHandles.GpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetGPUDescriptorHandleForHeapStart(), ImGuiHandleOffset, CbvDescriptorSize);

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
				CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetGPUDescriptorHandleForHeapStart(), 0, CbvDescriptorSize),
				CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetCPUDescriptorHandleForHeapStart(),  0, CbvDescriptorSize),
				false
			};
		}

		return {
			 CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetGPUDescriptorHandleForHeapStart(), offset, CbvDescriptorSize),
			 CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetCPUDescriptorHandleForHeapStart(),  offset, CbvDescriptorSize)
		};
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12MemoryManager::GetConstantBufferViewCpu() const
	{
		return CbvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12MemoryManager::GetConstantBufferViewGpu() const
	{
		return CbvHeap->GetGPUDescriptorHandleForHeapStart();
	}



}

