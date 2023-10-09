#pragma once
#include <Framework/Renderer/Api/GraphicsContext.h>
#include <Framework/Renderer/Settings/Settings.h>

#include "Platform/DirectX12/Heap/D3D12HeapManager.h"
#include "Platform/DirectX12/Resources/D3D12RenderFrame.h"
#include "Platform/DirectX12/DirectX12.h"


namespace Foundation
{
	class WindowsWindow;
}


#ifdef CM_DEBUG
#define NAME_D3D12_OBJECT(O, name)\
	O->SetName(name);
#else 
#define NAME_D3D12_OBJECT(O, name)
#endif


namespace Foundation::Graphics::D3D12
{
	
	class D3D12Context : public GraphicsContext
	{
	public:
		D3D12Context() = default;

		explicit D3D12Context(WindowsWindow* window);

		// @brief Disables copy and move semantics.
		DISABLE_COPY_AND_MOVE(D3D12Context);

		~D3D12Context() override;

		// @brief Initialises the core D3D engine, creating an interface to the Device, the main rendering Queue and a back up graphics command list with an accompanying allocator.
		void Init() override;

		// @brief Cleans up the core D3D resources, safely processing the last of the frame's deferred resources and unbinding any remaining interfaces to hardware.
		void ShutDown() override;

		// @brief Presents and swaps the main swap chain buffers.
		void SwapBuffers() override;

		// @brief Resizes the swap chain buffers to the specified dimensions.
		void OnResizeSwapChain(INT32 x, INT32 y) override;

		// @brief Executes all recorded commands and waits until the GPU
		//		  reaches the current fence point. Blocks CPU until the event
		//		  has been reached.
		void FlushRenderQueue();

		// @brief Executes all recorded commands but does not block further CPU
		//		  processes.
		void ExecuteGraphicsCommandList() const;

		[[nodiscard]] ID3D12CommandAllocator* GetGraphicsCommandAllocator() const { return pCmdAlloc.Get(); }

		// @brief Returns a pointer to the main rendering queue.
		[[nodiscard]] ID3D12CommandQueue* GetRenderingQueue() const { return pQueue.Get(); }

		// @brief Returns a pointer to the rendering command list.
		[[nodiscard]] ID3D12GraphicsCommandList* GetGraphicsCommandList() const { return pGraphicsCommandList.Get(); }

		// @brief Returns a pointer to the mesh shader command list. Note, mesh ShaderLib must be supported on your machine.
		[[nodiscard]] ID3D12GraphicsCommandList6* GetMeshShaderCommandList() const { return pDxmCommandList.Get(); }

		HRESULT InitD3D12Core();
		void CacheMSAAQuality();

		// @brief Marks resources for deletion.
		void SetDeferredReleasesFlag();

		// @brief Defers the release of resources until the start of the next frame.
		// @param[in] A pointer to the desired resource.
		void DeferredRelease(IUnknown* resource);

		// @brief Frees deferred resources with respect to the current frame resource.
		// @param[in] Takes the index of the current frame resource.
		void __declspec(noinline) ProcessDeferrals(UINT32 frame);

		D3D12DescriptorHeap* GetRTVHeap();
		D3D12DescriptorHeap* GetDSVHeap();
		D3D12DescriptorHeap* GetSRVHeap();
		D3D12DescriptorHeap* GetUAVHeap();

		// @brief Increments the sync counter.
		void IncrementSync();

		// @brief Switches to the next frame resource. Blocks CPU processes if
		//		  the next frame is currently in flight on the GPU.
		void IncrementFrame();

		// @brief Returns a pointer to the current frame resource.
		D3D12RenderFrame* CurrentRenderFrame();

		//	@brief Returns a pointer to the main device interface.
		[[nodiscard]] ID3D12Device* GetDevice() const;

		// @brief Returns a pointer to the main fence, associated with the
		//		  main render queue.
		[[nodiscard]] ID3D12Fence* GetFence() const;

		// @brief Returns a pointer to the swap chain buffer
		[[nodiscard]] IDXGISwapChain* GetSwapChain() { return pSwapChain.Get(); }

		// @brief Returns the current frame index.
		[[nodiscard]] UINT GetCurrentFrameIndex() const;

		// @brief Returns the current msaa quality flag.
		[[nodiscard]] UINT32 GetMSAAQuality();

		// @brief Returns the msaa state flag.
		[[nodiscard]] BOOL GetMSAAState();

		// @brief Returns a pointer to the main registered system window.
		[[nodiscard]] HWND   GetHwnd() const;

		// @brief Returns the current sync count associated with the core render queue.
		[[nodiscard]] UINT64 GetSyncCount() const;

		std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	private:
		void CreateCommandObjects();
		void CreateSwapChain();

		ComPtr<ID3D12GraphicsCommandList>	pGraphicsCommandList{ nullptr };
		ComPtr<ID3D12CommandQueue>			pQueue{ nullptr };
		ComPtr<ID3D12CommandAllocator>		pCmdAlloc{ nullptr };

		ComPtr<ID3D12Device>				pDevice			{ nullptr };
		ComPtr<IDXGIFactory4>				pDXGIFactory4	{ nullptr };
		ComPtr<ID3D12Fence>					pFence			{ nullptr };
		ComPtr<IDXGISwapChain>				pSwapChain		{ nullptr };

		D3D12DescriptorHeap	RtvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV };
		D3D12DescriptorHeap	DsvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV };
		D3D12DescriptorHeap	SrvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
		D3D12DescriptorHeap	UavHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };

		ComPtr<ID3D12Device8> pDxmDevice						{ nullptr };
		ComPtr<ID3D12GraphicsCommandList6> pDxmCommandList		{ nullptr };

		std::array<D3D12RenderFrame, FRAMES_IN_FLIGHT> RenderFrames;
		UINT32 FrameIndex{ 0 };

		std::vector<IUnknown*> DeferredReleases[FRAMES_IN_FLIGHT]{};
		UINT32 DeferralFlags[FRAMES_IN_FLIGHT];
		std::mutex DeferralsMutex;

		UINT64 SyncCounter{0};
		INT32 SwapChainWidth{1920};
		INT32 SwapChainHeight{1080};
		UINT32 MsaaQaulity = 0;
		BOOL MsaaState = false;
		D3D_DRIVER_TYPE DriverType{ D3D_DRIVER_TYPE_HARDWARE };
		HWND pWindowHandle{nullptr};

	public:/*...Debug layer...*/
		void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
		void LogAdapterOutputs(IDXGIAdapter* adapter);
		void LogAdapters();

		ComPtr<ID3D12Debug> DebugController;

	
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
				DeferredRelease(resource);
				resource = nullptr;
			}
		}

	};
}


