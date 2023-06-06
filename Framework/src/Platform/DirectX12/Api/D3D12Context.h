#pragma once
#include "Framework/Renderer/Api/GraphicsContext.h"
#include "Platform/DirectX12/Core/D3D12Core.h"

namespace Foundation::Graphics::D3D12
{

	class D3D12Context : public GraphicsContext
	{
	public:
		D3D12Context(HWND windowHandle, INT32 swapChainBufferWidth, INT32 swapChainBufferHeight);
		D3D12Context(const D3D12Context& other);
	
		~D3D12Context() override;

		void Init() override;
		void Clean() override;
		void SwapBuffers() override{}
		void FlushCommandQueue();
		void ExecuteGraphicsCommandList() const;

		ComPtr<IDXGISwapChain>				pSwapChain;
		ComPtr<ID3D12GraphicsCommandList4>	pGCL;
		ComPtr<ID3D12CommandQueue>			pQueue;
		ComPtr<ID3D12CommandAllocator>		pCmdAlloc;
		ComPtr<ID3D12Fence>					pFence;

		[[nodiscard]] UINT32 GetMsaaQaulity() const { return MsaaQaulity; }
		[[nodiscard]] bool GetMsaaState() const { return MsaaState; }

		// @brief Tracks the number of syncs between CPU and GPU.
		UINT64 SyncCounter;


	public:/*...Getters...*/
		[[nodiscard]] HWND GetHwnd() const { return pWindowHandle; }


	private:

		// @brief Creates the command object responsible for recording commands to be sent to GPU.
		void CreateCommandObjects();
		// @brief Creates the object responsible for creating resources to be used by the GPU.
		void CreateDevice();
		// @brief Creates the pointers responsible for swapping between the front and back buffers.
		void CreateSwapChain();
		// @brief Checks the MSAA qaulity support and caches level.
		void CheckMSAAQualityAndCache();

		// @brief Unsigned integer representing the supported multi sampling quality.
		UINT32 MsaaQaulity = 0;
		bool MsaaState = false;

		// @brief This is out dated. Please use frame buffer class instead.   
		INT32 SwapChainWidth;
		INT32 SwapChainHeight;

		D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;
		HWND pWindowHandle;

	public:/*...Debug layer...*/
		void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
		void LogAdapterOutputs(IDXGIAdapter* adapter);
		void LogAdapters();

		ComPtr<ID3D12Debug> DebugController;

		HMODULE GpuCaptureLib;
		HMODULE GpuTimingLib;

	};
}


