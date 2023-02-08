#pragma once
#include "Framework/Renderer/Memory/MemoryManager.h"
#include "Platform/DirectX12/Buffers/D3D12BufferUtils.h"


namespace Engine
{

	class D3D12Context;

	class D3D12MemoryManager : public MemoryManager
	{
		struct Handles
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE GpuCurrentHandle;
			CD3DX12_CPU_DESCRIPTOR_HANDLE CpuCurrentHandle;
		};
	public:

		D3D12MemoryManager() = default;
		~D3D12MemoryManager() override;

		void Allocate(UINT64 ptr, UINT64 memSize) override;
		void Deallocate(UINT64 ptr, UINT64 memSize) override;

		HRESULT InitialiseSrvUavHeap(D3D12Context* context, UINT64 size);

		Handles GetResourceHandle(INT32 offset = 1);

		[[nodiscard]] UINT GetResourceHeapSize() const { return SrvUavDescriptorSize; }

		[[nodiscard]] ID3D12DescriptorHeap* GetDescriptorHeap() const { return SrvUavHeap.Get(); }



	private:
		bool IsInitialised = false;
		ComPtr<ID3D12DescriptorHeap> SrvUavHeap = nullptr;
		Handles ResourceHandles;

		UINT SrvUavDescriptorSize;
		UINT64 SizeAllocated;
		UINT64 HandleOffset = 0;
	};
}
