#pragma once
#include <Framework/Renderer/Api/GraphicsContext.h>
#include "Platform/DirectX12/Heap/D3D12HeapManager.h"
#include "Platform/DirectX12/Constants/D3D12GlobalConstants.h"
#include "Platform/DirectX12/Resources/D3D12RenderFrame.h"
#include "Platform/DirectX12/DirectX12.h"
#include <array>


namespace Foundation
{
	class Win32Window;
}


#ifdef CM_DEBUG
#define NAME_D3D12_OBJECT(O, name)\
	O->SetName(name);
#else 
#define NAME_D3D12_OBJECT(O, name)
#endif


namespace Foundation::Graphics::D3D12
{
	namespace Internal
	{
		void DeferredRelease(IUnknown* resource);



	}
	void __declspec(noinline) ProcessDeferrals(UINT32 frame);


	HRESULT InitD3D12Core();
	void CacheMSAAQuality();
	void Shutdown();


	constexpr D3D12DescriptorHeap* GetRTVHeap();
	constexpr D3D12DescriptorHeap* GetDSVHeap();
	constexpr D3D12DescriptorHeap* GetSRVHeap();
	constexpr D3D12DescriptorHeap* GetUAVHeap();

	constexpr ID3D12Device8* Device();
	constexpr D3D12RenderFrame* CurrentRenderFrame();
	constexpr ID3D12Fence* Fence();

	void SetDeferredReleasesFlag();
	D3D12RenderFrame* IncrementFrame();

	template<typename T> constexpr void Release(T*& resource)
	{
		if (resource)
		{
			resource->Release();
			resource = nullptr;
		}
	}

	template<typename T> constexpr void DeferredRelease(T*& resource)
	{
		if (resource)
		{
			Internal::DeferredRelease(resource);
			resource = nullptr;
		}
	}

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();


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




	public:/*...Getters...*/
		[[nodiscard]] HWND GetHwnd() const;
		[[nodiscard]] UINT64 GetSyncCount() const;

	private:
		void CreateCommandObjects();
		void CreateSwapChain();


		UINT64 SyncCounter{0};
		INT32 SwapChainWidth{1920};
		INT32 SwapChainHeight{1080};

		D3D_DRIVER_TYPE DriverType{ D3D_DRIVER_TYPE_HARDWARE };
		HWND pWindowHandle{nullptr};

	public:/*...Debug layer...*/
		void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
		void LogAdapterOutputs(IDXGIAdapter* adapter);
		void LogAdapters();

		ComPtr<ID3D12Debug> DebugController;

		HMODULE GpuCaptureLib;
		HMODULE GpuTimingLib;

	};
}


