#pragma once
#include "../DirectX12.h"
#include "Framework/Renderer/Api/GraphicsContext.h"

using Microsoft::WRL::ComPtr;

namespace Engine
{

	class D3D12Context : public GraphicsContext
	{
	public:
		[[nodiscard]] HWND GetHwnd() const { return WindowHandle; }

	public:

		// Debugging layer

		void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

		void LogAdapterOutputs(IDXGIAdapter* adapter);

		void LogAdapters();

	public:

		D3D12Context(HWND windowHandle, INT32 swapChainBufferWidth, INT32 swapChainBufferHeight);
		D3D12Context(const D3D12Context& other);
	
		~D3D12Context() override;

		void Init() override;

		void SwapBuffers() override{}
		void FlushCommandQueue();
		void ExecuteGraphicsCommandList() const;
		void SignalGPU() const;

		// @brief Create the descriptors for the constant buffer view and resource view.
		bool CreateCbvSrvUavHeap
		(
			UINT opaqueRenderItemCount,
			UINT transparentRenderItemCount,
			UINT voxelWorldResources,
			UINT frameResourceCount
		);

		ComPtr<ID3D12Device>				Device;
		ComPtr<IDXGISwapChain>				SwapChain;
		ComPtr<IDXGIFactory4>				DXGIFactory;
		ComPtr<ID3D12GraphicsCommandList>	GraphicsCmdList;
		ComPtr<ID3D12CommandQueue>			CommandQueue;
		ComPtr<ID3D12CommandAllocator>		CmdListAlloc;
		ComPtr<ID3D12Fence>					Fence;
		ComPtr<ID3D12RootSignature>			RootSignature;
		ComPtr<ID3D12Resource>				DepthStencilBuffer;

		[[nodiscard]] UINT GetPassConstBufferViewOffset() const { return PassConstantBufferViewOffset; }
		[[nodiscard]] UINT32 GetMsaaQaulity() const { return MsaaQaulity; }
		[[nodiscard]] bool GetMsaaState() const { return MsaaState; }

		ComPtr<ID3D12DescriptorHeap> CbvHeap;

		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetConstantBufferViewCpu() const;
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetConstantBufferViewGpu() const;

		
		UINT32 CbvSrvUavDescriptorSize;
		UINT VoxelWorld_Resource_HeapOffset;

		// @brief Tracks the number of syncs between CPU and GPU.
		UINT64 GPU_TO_CPU_SYNC_COUNT;


		UINT PassConstantBufferViewOffset;

	private:

		// @brief Creates the command object responsible for recording commands to be sent to the
		//		  GPU.
		bool CreateCommandObjects();

		// @brief Creates the object responsible for creating resources to be used by the GPU.
		bool CreateDevice();

		// @brief Creates the pointers responsible for swapping between the front and back buffers.
		bool CreateSwapChain();

		// @brief Checks the MSAA qaulity support and caches level.
		bool CheckMSAAQualityAndCache();

	

	
		bool BuildRootSignature();


		// @brief Unsigned integer representing the supported multi sampling quality.
		UINT32 MsaaQaulity = 0;
		bool MsaaState = false;

		// @brief This is out dated. Please use frame buffer class instead.   
		INT32 SwapChainBufferWidth;
		INT32 SwapChainBufferHeight;

		D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;


	private:
		HWND WindowHandle;

	};
}

