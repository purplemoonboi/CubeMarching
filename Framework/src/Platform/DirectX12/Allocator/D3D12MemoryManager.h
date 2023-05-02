#pragma once
#include "Framework/Renderer/Memory/MemoryManager.h"
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"


namespace Engine
{

	class D3D12Context;

	class D3D12MemoryManager : public MemoryManager
	{
		struct Handles
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE GpuCurrentHandle;
			CD3DX12_CPU_DESCRIPTOR_HANDLE CpuCurrentHandle;
			bool IsValid = true;
		};

		struct ImGuiHandles
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE GpuHandle;
			CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle;
		};

	public:

		D3D12MemoryManager(ID3D12Device* device);
		~D3D12MemoryManager();

		void Allocate(UINT64 ptr, UINT64 memSize)	override;

		void Deallocate(UINT64 ptr, UINT64 memSize) override;

		HRESULT InitialiseCbvHeap(ID3D12Device* device, UINT64 frameResourcesCount, UINT64 maxObjectCount);

		HRESULT InitialiseSrvUavHeap(ID3D12Device* device, UINT64 size);

		Handles GetResourceHandle(INT32 offset = 1);


		[[nodiscard]] UINT GetDescriptorIncrimentSize() const { return CbvDescriptorSize; }

		[[nodiscard]] ID3D12DescriptorHeap* GetConstantBufferDescHeap() const { return CbvHeap.Get(); }

		[[nodiscard]] ID3D12DescriptorHeap* GetShaderResourceDescHeap() const { return SrvUavHeap.Get(); }

		[[nodiscard]] UINT64 GetCurrentHandleOffset() const { return HandleOffset; }

		[[nodiscard]] const ImGuiHandles* GetImGuiHandles() const { return &ImGuiHandles; }

		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetConstantBufferHandle() const;
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetConstantBufferGpuHandle() const;

		[[nodiscard]] UINT GetPassBufferOffset() const { return PassBufferOffsetSize; }

	private:
		bool IsInitialised = false;
		ComPtr<ID3D12DescriptorHeap> CbvHeap = nullptr;
		ComPtr<ID3D12DescriptorHeap> SrvUavHeap = nullptr;


		Handles ResourceHandles;
		ImGuiHandles ImGuiHandles;

		UINT CbvDescriptorSize;
		UINT PassBufferOffsetSize;

		UINT SrvUavDescriptorSize;
		UINT64 SizeAllocated;
		UINT64 HandleOffset = 0;

		UINT64 MaxHandleOffset = 0;
		UINT64 ImGuiHandleOffset = 0;
	};
}
