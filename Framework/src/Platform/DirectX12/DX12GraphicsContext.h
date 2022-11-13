#pragma once
#include "DirectX12.h"
#include "Framework/Renderer/GraphicsContext.h"

using Microsoft::WRL::ComPtr;

namespace Engine
{

	// @brief Double buffering
	static constexpr  INT32 SWAP_CHAIN_BUFFER_COUNT = 2;

	class DX12GraphicsContext : public GraphicsContext
	{
	public:

		DX12GraphicsContext(HWND windowHandle, INT32 swapChainBufferWidth, INT32 swapChainBufferHeight);
		DX12GraphicsContext(const DX12GraphicsContext&) = delete;
		DX12GraphicsContext(DX12GraphicsContext&&) = delete;
		auto operator=(const DX12GraphicsContext&) noexcept -> DX12GraphicsContext & = delete;
		auto operator=(DX12GraphicsContext&&) noexcept -> DX12GraphicsContext && = delete;

		~DX12GraphicsContext() override;

		void Init() override;

		void SwapBuffers() override{}

		void FlushCommandQueue();

		void SetBackBufferIndex(INT32 value) { BackBufferIndex = value; }

		ID3D12Resource* CurrentBackBuffer() const;
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE ConstantBufferView() const;

		ComPtr<ID3D12Device>				Device;
		ComPtr<IDXGISwapChain>				SwapChain;
		ComPtr<IDXGIFactory4>				DXGIFactory;
		ComPtr<ID3D12GraphicsCommandList>	GraphicsCmdList;
		ComPtr<ID3D12CommandQueue>			CommandQueue;
		ComPtr<ID3D12CommandAllocator>		CmdListAlloc;
		ComPtr<ID3D12Fence>					Fence;
		ComPtr<ID3D12RootSignature>			RootSignature;
		ComPtr<ID3D12Resource>				SwapChainBuffer[SWAP_CHAIN_BUFFER_COUNT];
		ComPtr<ID3D12Resource>				DepthStencilBuffer;

		ComPtr<ID3D12PipelineState> CurrentPso = nullptr;


		// @brief Heap descriptor for resources
		ComPtr<ID3D12DescriptorHeap> RtvHeap;
		// @brief Heap descriptor for depth-stencil resource
		ComPtr<ID3D12DescriptorHeap> DsvHeap;
		// @brief Heap descriptor for constant buffer resource
		ComPtr<ID3D12DescriptorHeap> CbvHeap;


		// @brief - Returns the formatting of the back buffer.
		DXGI_FORMAT GetBackBufferFormat() const { return BackBufferFormat; }
		// @brief - Returns the formatting of the depth stencil.
		DXGI_FORMAT GetDepthStencilFormat() const { return DepthStencilFormat; }

		// @brief - Returns the size of the RTV descriptor size.
		UINT32 GetRtvDescSize() const { return RtvDescriptorSize; }

		// @brief - Returns the size of the DSV descriptor size.
		UINT32 GetDsvDescSize() const { return DsvDescriptorSize; }

		UINT32 GetMsaaQaulity() const { return MsaaQaulity; }

		bool GetMsaaState() const { return MsaaState; }

		void UpdateBackBufferIndex(INT32 index) { BackBufferIndex = index; }

		INT32 GetBackBufferIndex() const { return BackBufferIndex; }



		// Debugging layer

		void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

		void LogAdapterOutputs(IDXGIAdapter* adapter);

		void LogAdapters();

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

		// @brief Creates descriptors for the depth-stencil, render texture and constant buffers.
		bool CreateDsvRtvAndCbvDescriptorHeaps();

		bool BuildRootSignature();

		// @brief - Represents the size of the RTV descriptor heap.
		UINT32 RtvDescriptorSize;
		// @brief - Represents the size of the DSV descriptor heap.
		UINT32 DsvDescriptorSize;
		// @brief - Represents the size of the CBV, SRV and UAV descriptor heaps.
		UINT32 CbvSrvUavDescriptorSize;

		// @brief Tracks the number of syncs between CPU and GPU.
		UINT64 GPU_TO_CPU_SYNC_COUNT = 0;


		// @brief Index to the current back buffer
		INT32 BackBufferIndex = 0;
		// @brief Defines the format of the back buffer.
		DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		// @brief Defines the format of the stencil.
		DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;


		// @brief Unsigned integer representing the supported multi sampling quality.
		UINT32 MsaaQaulity = 0;
		bool MsaaState = false;

		// @brief - A handle to the window
		HWND WindowHandle;


		// @brief This is out dated. Please use frame buffer class instead.   
		INT32 SwapChainBufferWidth;
		INT32 SwapChainBufferHeight;

		D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;
	};
}


