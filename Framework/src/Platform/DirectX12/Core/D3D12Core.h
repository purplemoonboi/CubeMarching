#pragma once
#include "Platform/DirectX12/DirectX12.h"
#include "Platform/DirectX12/Heap/D3D12HeapManager.h"

using Microsoft::WRL::ComPtr;

#ifdef CM_DEBUG
#define NAME_D3D12_OBJECT(O, name)\
	O->SetName(name);
#else 
#define NAME_D3D12_OBJECT()
#endif

namespace Foundation::Graphics::D3D12
{
	// Global variables

	inline constexpr INT32 SWAP_CHAIN_BUFFER_COUNT = 2;
	inline constexpr INT32 FRAMES_IN_FLIGHT = 1;

	inline ComPtr<ID3D12Device8>					pDevice{ nullptr };
	inline ComPtr<IDXGIFactory4>				pDXGIFactory4{ nullptr };
	inline UINT32 FrameIndex{ 0 };

	inline UINT32 DeferralFlags[FRAMES_IN_FLIGHT];
	inline std::mutex DeferralsMutex;


	inline D3D12DescriptorHeap	RtvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV };
	inline D3D12DescriptorHeap	DsvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV };
	inline D3D12DescriptorHeap	SrvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
	inline D3D12DescriptorHeap	UavHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };

	inline std::vector<IUnknown*> DeferredReleases[FRAMES_IN_FLIGHT]{};

	// Global functions
	namespace Internal
	{
		void DeferredRelease(IUnknown* resource);
	}

	inline void SetDeferredReleasesFlag() { DeferralFlags[FrameIndex] = 1; }

	template<typename T> constexpr void Release(T*& resource)
	{
		if(resource)
		{
			resource->Release();
			resource = nullptr;
		}
	}

	template<typename T> constexpr void DeferredRelease(T*& resource)
	{
		if (resource)
		{
			Internal::DeferredRelease(resource);
			resource = nullptr;
		}
	}
}