#pragma once
#include "Framework/Core/Core.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"


namespace Foundation
{
	using Microsoft::WRL::ComPtr;

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
		D3D12HeapManager() = default;
		D3D12HeapManager(ID3D12Device* device);
		DISABLE_COPY_AND_MOVE(D3D12HeapManager);
		~D3D12HeapManager();


		HRESULT Init(UINT framesInFlight, UINT maxObjectCount);

		Handles GetResourceHandle(INT32 offset = 1);

		[[nodiscard]] UINT GetCbvSrvUavDescSize()	const { return CbvSrvUavDescriptorSize; }
		[[nodiscard]] UINT GetRtvDescSize()			const { return RtvDescriptorSize; }
		[[nodiscard]] UINT GetDsvDescSize()			const { return DsvDescriptorSize; }

		[[nodiscard]] ID3D12DescriptorHeap* GetConstantBufferDescHeap() const { return CbvHeap.Get(); }
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetConstantBufferViewCpu() const;
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetConstantBufferViewGpu() const;

		[[nodiscard]] ID3D12DescriptorHeap* GetShaderResourceDescHeap() const { return SrvUavHeap.Get(); }

		[[nodiscard]] ID3D12DescriptorHeap* GetRenderTargetHeap()		const { return RtvHeap.Get(); }
		[[nodiscard]] ID3D12DescriptorHeap* GetDepthStencilHeap()		const { return DsvHeap.Get(); }

		[[nodiscard]] UINT64 GetCurrentHandleOffset() const { return HandleOffset; }

		[[nodiscard]] const ImGuiHandles* GetImGuiHandles() const { return &ImGuiHandles; }



		[[nodiscard]] UINT GetPassBufferOffset() const { return PassBufferOffsetSize; }

	private:
		HRESULT CreateRtvAndDsvHeaps();

	private:

		ID3D12Device* pDevice;
		bool IsInitialised = false;



		Handles ResourceHandles;
		ImGuiHandles ImGuiHandles;


		CD3DX12_GPU_DESCRIPTOR_HANDLE NullHandle;

		UINT CbvSrvUavDescriptorSize;
		// @brief - Represents the size of the RTV descriptor heap.
		UINT RtvDescriptorSize;
		// @brief - Represents the size of the DSV descriptor heap.
		UINT DsvDescriptorSize;

		UINT PassBufferOffsetSize;

		UINT64 HandleOffset = 0;

		UINT64 MaxHandleOffset = 0;
		UINT64 ImGuiHandleOffset = 0;


		// @brief A pointer to the Cbv Heap.
		ComPtr<ID3D12DescriptorHeap> CbvHeap = nullptr;

		// @brief A pointer to the SrvUav Heap.
		ComPtr<ID3D12DescriptorHeap> SrvUavHeap = nullptr;

		// @brief Heap descriptor for resources.
		ComPtr<ID3D12DescriptorHeap> RtvHeap;

		// @brief Heap descriptor for depth-stencil resource.
		ComPtr<ID3D12DescriptorHeap> DsvHeap;
	};
}
