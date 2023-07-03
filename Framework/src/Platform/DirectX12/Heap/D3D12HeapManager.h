#pragma once
#include "Framework/Core/Core.h"
#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/DirectX12.h"
#include "Platform/DirectX12/Constants/D3D12GlobalConstants.h"





using Microsoft::WRL::ComPtr;

namespace Foundation::Graphics::D3D12
{

	struct D3D12DescriptorHandle
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE GpuHandle{};
		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle{};
		bool IsValid{ true };
		UINT32 Index{ 0 };
	};


	class D3D12DescriptorHeap
	{
	public:
		explicit D3D12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);
		DISABLE_COPY_AND_MOVE(D3D12DescriptorHeap);
		~D3D12DescriptorHeap() {  };

		HRESULT Init(UINT32 capacity, bool isShaderVisible);
		void Release();

		[[nodiscard]] D3D12DescriptorHandle Allocate();
		void Free(D3D12DescriptorHandle& pHandle);

		void ProcessDeferredFree(UINT32 frame);

	public:/*...Getters...*/

		[[nodiscard]] D3D12_DESCRIPTOR_HEAP_TYPE	GetType()		 const { return Type; }
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE	BeginCPU()		 const { return pBeginCPU; }
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE	BeginGPU()		 const { return pBeginGPU; }
		[[nodiscard]] ID3D12DescriptorHeap*			GetHeap()		 const { return pHeap.Get(); }
		[[nodiscard]] UINT32						GetCapacity()	 const { return Capacity; }
		[[nodiscard]] UINT32						GetSize()		 const { return Size; }
		[[nodiscard]] UINT32						DescriptorSize() const { return HeapSize; }


	private:

		ComPtr<ID3D12DescriptorHeap> pHeap{ nullptr };
		const D3D12_DESCRIPTOR_HEAP_TYPE Type;

		D3D12_CPU_DESCRIPTOR_HANDLE pBeginCPU{0};
		D3D12_GPU_DESCRIPTOR_HANDLE pBeginGPU{0};

		UINT HeapSize{ 0U };	// Heap size in bytes.
		UINT Capacity{ 0U };	// Total number of allocations available.
		UINT Size{ 0U };		// Number of allocations.

		ScopePointer<UINT32[]> AvailableHandles{nullptr};
		std::vector<UINT32>    DeferredAvailableIndices[FRAMES_IN_FLIGHT];

		bool IsShaderVisible{ true };

		std::mutex Mutex{};
	};

}
