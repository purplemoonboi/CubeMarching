#pragma once
#include <Framework/Renderer/Api/GraphicsContext.h>

#include "Platform/DirectX12/DirectX12.h"


namespace Foundation
{
	class Win32Window;
}

namespace Foundation::Graphics::D3D12
{

	class D3D12Context : public GraphicsContext
	{
	public:
		explicit D3D12Context(Win32Window* window);
		DISABLE_COPY_AND_MOVE(D3D12Context);
		~D3D12Context() override;

		void Init() override;
		void Clean() override;
		void SwapBuffers() override{}
		void FlushCommandQueue();
		void ExecuteGraphicsCommandList() const;

		ComPtr<IDXGISwapChain>				pSwapChain	{nullptr};
		ComPtr<ID3D12GraphicsCommandList4>	pGCL		{nullptr};
		ComPtr<ID3D12CommandQueue>			pQueue		{nullptr};
		ComPtr<ID3D12CommandAllocator>		pCmdAlloc	{nullptr};
		ComPtr<ID3D12Fence>					pFence		{nullptr};


		// @brief Tracks the number of syncs between CPU and GPU.
		UINT64 SyncCounter;


	public:/*...Getters...*/
		[[nodiscard]] HWND GetHwnd() const { return pWindowHandle; }


	private:

		// @brief Creates the command object responsible for recording commands to be sent to GPU.
		void CreateCommandObjects();
		// @brief Creates the pointers responsible for swapping between the front and back buffers.
		void CreateSwapChain();
		

		// @brief This is out dated. Please use frame buffer class instead.   
		INT32 SwapChainWidth{1920};
		INT32 SwapChainHeight{1080};

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


