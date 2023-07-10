#include "D3D12HeapManager.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/D3D12/D3D12.h"

namespace Foundation::Graphics::D3D12
{
	D3D12DescriptorHeap::D3D12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type)
		:
		Type(type)
	{
	}

	HRESULT D3D12DescriptorHeap::Init(UINT32 capacity, bool isShaderVisible)
	{
		std::lock_guard lock{ Mutex };

		CORE_ASSERT((capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2), "Invalid capacity!");
		CORE_ASSERT(!(Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) && capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE, "Invalid heap type!");

		if(Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || Type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
		{
			IsShaderVisible = false;
		}

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Flags = (IsShaderVisible) ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NumDescriptors = capacity;
		desc.Type = Type;
		desc.NodeMask = 0;

		ID3D12Device8* device = gD3D12Context->GetDevice();

		HRESULT hr{ S_OK };
		hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap));
		if(!SUCCEEDED(hr))
		{
			Release();
			return hr;
		}

		AvailableHandles = CreateScope<UINT32[]>(capacity);
		Capacity = capacity;
		Size = 0;
		IsShaderVisible = isShaderVisible;

		for(UINT32 i{0}; i < capacity; ++i)
		{
			AvailableHandles[i] = i;
		}

		for(UINT32 i{0}; i < FRAMES_IN_FLIGHT; ++i)
		{
			CORE_ASSERT(DeferredAvailableIndices[i].empty(), " ");
		}

		HeapSize = device->GetDescriptorHandleIncrementSize(Type);

		pBeginCPU = pHeap->GetCPUDescriptorHandleForHeapStart();
		pBeginGPU = (isShaderVisible) ? pHeap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };


		return hr;
	}

	void D3D12DescriptorHeap::Release()
	{
		CORE_ASSERT(!Size, "Descriptors still live!");
		gD3D12Context->DeferredRelease(pHeap.Get());
	}

	D3D12DescriptorHandle D3D12DescriptorHeap::Allocate()
	{
		std::lock_guard{ Mutex };

		CORE_ASSERT(pHeap.Get(), "Invalid heap!");
		CORE_ASSERT(Size > Capacity, "Out of memory!");

		const UINT32 index = AvailableHandles[Size];
		const UINT32 offset = index * HeapSize;
		++Size;

		D3D12DescriptorHandle pHandle;
		pHandle.CpuHandle.ptr = pBeginCPU.ptr + offset;
		pHandle.Index = index;

		if(IsShaderVisible)
		{
			pHandle.GpuHandle.ptr = pBeginGPU.ptr + offset;
		}

		return pHandle;
	}

	void D3D12DescriptorHeap::Free(D3D12DescriptorHandle& pHandle)
	{
		if(!pHandle.IsValid)
		{
			return;
		}
		std::lock_guard{ Mutex };

		CORE_ASSERT(pHandle.CpuHandle.ptr >= pBeginCPU.ptr, "Invalid handle!");
		CORE_ASSERT((pHandle.CpuHandle.ptr - pBeginCPU.ptr) % HeapSize == 0, "Incorrect type of handle!");
		CORE_ASSERT(pHandle.Index > Capacity, "Index exceeded max descriptor count!");
		const auto index = static_cast<UINT32>((pHandle.CpuHandle.ptr - pBeginCPU.ptr) / HeapSize);
		CORE_ASSERT(pHandle.Index == index, "Index does not match the index of this descriptor slot!");

		// Defer the free operation until we know it is safe.
		DeferredAvailableIndices[gD3D12Context->CurrentFrameIndex()].push_back(index);

		// Let the renderer know there are pending releases for the current frame.
		gD3D12Context->SetDeferredReleasesFlag();
		pHandle = {};

	}

	void D3D12DescriptorHeap::ProcessDeferredFree(UINT32 frame)
	{
		std::lock_guard{ Mutex };
		CORE_ASSERT(frame < FRAMES_IN_FLIGHT, "Invalid frame index!");

		std::vector<UINT32>& indices{ DeferredAvailableIndices[frame] };
		if(!indices.empty())
		{
			for(auto index : indices)
			{
				--Size;
				AvailableHandles[Size] = index;
			}
			indices.clear();
		}
	}
}

