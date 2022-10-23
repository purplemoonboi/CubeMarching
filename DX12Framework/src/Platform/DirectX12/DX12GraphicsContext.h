#pragma once
#include "DirectX12.h"
#include "Framework/Renderer/GraphicsContext.h"

namespace DX12Framework
{
	static constexpr  INT32 SwapChainBufferCount = 2;


	class DX12GraphicsContext : public GraphicsContext
	{
	public:
		DX12GraphicsContext
		(
			HWND windowHandle,
			INT32 viewportWidth,
			INT32 viewportHeight
		);

		virtual ~DX12GraphicsContext();

		void Init() override;

		void SwapBuffers() override;

		void ResetCommandList();

		void OpenCommandList();

		void CloseCommandList();

		void FlushCommandQueue();

		void Resize();

		void SetBackBufferIndex(UINT value) { BackBufferIndex = value; }

		ID3D12Resource* CurrentBackBuffer() const;
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;




		Microsoft::WRL::ComPtr<ID3D12Device>	Device;
		Microsoft::WRL::ComPtr<IDXGISwapChain>  SwapChain;
		Microsoft::WRL::ComPtr<IDXGIFactory4>	DXGIFactory;

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>		  CommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>	  DirectCommandListAllocator;
		Microsoft::WRL::ComPtr<ID3D12Fence> D3DFence() { return SyncFence; }


		Microsoft::WRL::ComPtr<ID3D12Resource>			  SwapChainBuffer[SwapChainBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource>			  DepthStencilBuffer;

		// @brief
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RtvHeap;

		// @brief
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DsvHeap;

		// @brief - Returns the formatting of the back buffer.
		DXGI_FORMAT GetBackBufferFormat() const { return BackBufferFormat; }
		// @brief - Returns the formatting of the depth stencil.
		DXGI_FORMAT GetDepthStencilFormat() const { return DepthStencilFormat; }

		// @brief - Returns the size of the RTV descriptor size.
		UINT32 GetRtvDescSize() const { return RtvDescriptorSize; }

		// @brief - Returns the size of the DSV descriptor size.
		UINT32 GetDsvDescSize() const { return DsvDescriptorSize; }

		UINT32 GetMsaaQaulity() const { return MsaaQaulity; }


		void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

		void LogAdapterOutputs(IDXGIAdapter* adapter);

		void LogAdapters();

		void UpdateBackBufferIndex(INT32 index) { BackBufferIndex = index; }
		INT32 GetBackBufferIndex() const { return BackBufferIndex; }

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

		// @brief 
		bool CreateDsvDescriptorHeap();
		// @brief 
		bool CreateRtvDescriptorHeap();
		// @brief Creates the description for a resource buffer. (of type CBV, SRV and UAV)
		bool CreateCbvSrvUavDescriptorHeap();



		Microsoft::WRL::ComPtr<ID3D12Fence>				  SyncFence;

		// @brief Tracks the number of syncs between CPU and GPU.
		UINT64 FenceSyncCount = 0;

		// @brief Index to the current back buffer
		INT32 BackBufferIndex = 0;


		// @brief - Represents the size of the RTV descriptor heap.
		UINT32 RtvDescriptorSize;
		// @brief - Represents the size of the DSV descriptor heap.
		UINT32 DsvDescriptorSize;
		// @brief - Represents the size of the CBV, SRV and UAV descriptor heaps.
		UINT32 CbvSrvUavDescriptorSize;

		// @brief Unsigned integer representing the supported multi sampling quality.
		UINT32 MsaaQaulity;

		// @brief Defines the format of the back buffer.
		DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		// @brief Defines the format of the stencil.
		DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		HWND WindowHandle;
		INT32 ViewportWidth;
		INT32 ViewportHeight;

		D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;
	};
}


