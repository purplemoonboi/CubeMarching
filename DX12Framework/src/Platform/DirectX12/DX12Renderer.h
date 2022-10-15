#pragma once
#include "DirectX12.h"

#include "Framework/Core/Core.h"

namespace DX12Framework
{

	class DX12Renderer 
	{
	private:
		DX12Renderer(HWND windowHandle, INT32 bufferWidth, INT32 bufferHeight);
		DX12Renderer(const DX12Renderer&) = default;
		~DX12Renderer();

	public:

		static RefPointer<DX12Renderer> Create();

	private:

		// @brief Creates the object responsible for creating resources to be used by the GPU.
		bool CreateDevice();
		// @brief Checks the MSAA qaulity support and caches level.
		bool CheckMSAAQualityAndCache();
		// @brief Creates the command object responsible for recording commands to be sent to the
		//		  GPU.
		bool CreateCommandObjects();
		// @brief Creates the pointers responsible for swapping between the front and back buffers.
		bool CreateSwapChain();
		// @brief Creates the description for a resource buffer. (of type RTV)
		bool CreateRtvDescriptorHeap();
		// @brief Creates the description for a resource buffer. (of type DSV)
		bool CreateDsvDescriptorHeap();
		// @brief Creates the description for a resource buffer. (of type CBV, SRV and UAV)
		bool CreateCbvSrvUavDescriptorHeap();
	
		void Resize();

		Microsoft::WRL::ComPtr<IDXGISwapChain>  SwapChain;
		Microsoft::WRL::ComPtr<IDXGIFactory4>	DXGIFactory;

		Microsoft::WRL::ComPtr<ID3D12Device>			  Device;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>	  DirectCommandListAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>		  CommandQueue;
		Microsoft::WRL::ComPtr<ID3D12Fence>				  SyncFence;

		static const INT32 SwapChainBufferCount = 2;
		INT32 CurrentBackBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource>			  SwapChainBuffer[SwapChainBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource>			  DepthStencilBuffer;

		// @brief Tracks the number of syncs between CPU and GPU.
		UINT64 CurrentFence;

		// @brief - The following unsigned integers represent the size of descriptor types.
		//			Descriptor sizes vary across GPUs, so we need to get this information
		//			and cache them for later when we want to create descriptors.

		// @brief Represents the size of the RTV descriptor heap.
		UINT RtvDescriptorSize;
		// @brief Represents the size of the DSV descriptor heap.
		UINT DsvDescriptorSize;
		// @brief Represents the size of the CBV, SRV and UAV descriptor heaps.
		UINT CbvSrvUavDescriptorSize;

		// @brief
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RtvHeap;

		// @brief
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DsvHeap;

		// @brief Unsigned integer representing the supported multi sampling quality.
		UINT MSAA4xQaulity;

		// @brief Defines the D3D device type
		D3D_DRIVER_TYPE D3DDriverType;
		// @brief Defines the format of the back buffer.
		DXGI_FORMAT BackBufferFormat;
		// @brief Defines the format of the stencil.
		DXGI_FORMAT DepthStencilFormat;


		// @brief This is out dated. Please use frame buffer class instead.   
		INT32 BufferWidth;
		INT32 BufferHeight;

		HWND* ApplicationWindowHandle;


	};

}