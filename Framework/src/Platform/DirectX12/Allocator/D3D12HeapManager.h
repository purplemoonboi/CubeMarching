#pragma once
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"


namespace Engine
{

	class D3D12Context;

	class D3D12HeapManager 
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

		D3D12HeapManager(ID3D12Device* device);
		~D3D12HeapManager();

		HRESULT Init(UINT framesInFlight, UINT maxObjectCount);

		Handles GetResourceHandle(INT32 offset = 1);


		[[nodiscard]] UINT GetDescriptorIncrimentSize() const { return CbvSrvUavDescriptorSize; }

		[[nodiscard]] ID3D12DescriptorHeap* GetConstantBufferDescHeap() const { return CbvHeap.Get(); }

		[[nodiscard]] ID3D12DescriptorHeap* GetShaderResourceDescHeap() const { return SrvUavHeap.Get(); }

		[[nodiscard]] UINT64 GetCurrentHandleOffset() const { return HandleOffset; }

		[[nodiscard]] const ImGuiHandles* GetImGuiHandles() const { return &ImGuiHandles; }

		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetConstantBufferViewCpu() const;
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetConstantBufferViewGpu() const;

		[[nodiscard]] UINT GetPassBufferOffset() const { return PassBufferOffsetSize; }

	private:

		ID3D12Device* pDevice;


		bool IsInitialised = false;
		ComPtr<ID3D12DescriptorHeap> CbvHeap = nullptr;
		ComPtr<ID3D12DescriptorHeap> SrvUavHeap = nullptr;


		Handles ResourceHandles;
		CD3DX12_GPU_DESCRIPTOR_HANDLE NullHandle;
		ImGuiHandles ImGuiHandles;

		UINT CbvSrvUavDescriptorSize;
		UINT PassBufferOffsetSize;

		UINT SrvUavDescriptorSize;
		UINT64 SizeAllocated;
		UINT64 HandleOffset = 0;

		UINT64 MaxHandleOffset = 0;
		UINT64 ImGuiHandleOffset = 0;
	};
}
