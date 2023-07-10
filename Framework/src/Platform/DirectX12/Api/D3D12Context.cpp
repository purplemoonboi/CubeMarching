#include "Framework/cmpch.h"
#include "D3D12Context.h"

#include "Framework/Core/Log/Log.h"

#include "Platform/Windows/Win32Window.h"


namespace Foundation::Graphics::D3D12
{

	D3D12Context::D3D12Context(Win32Window* window)
		:
		pWindowHandle(static_cast<HWND>(window->GetNativeWindow())),
		SwapChainWidth(window->GetWidth()),
		SwapChainHeight(window->GetHeight())
	{
		CORE_ASSERT(pWindowHandle, "Window handle is null!");
	}

	D3D12Context::~D3D12Context()
	{
		if (pSwapChain != nullptr)
		{
			pSwapChain.Reset();
			pSwapChain = nullptr;
		}
	}

	HRESULT D3D12Context::InitD3D12Core()
	{
		HRESULT hr{ S_OK };

		// Initialise heaps.
		hr = RtvHeap.Init(512, false);
		THROW_ON_FAILURE(hr);
		hr = DsvHeap.Init(512, false);
		THROW_ON_FAILURE(hr);
		hr = SrvHeap.Init(4096, true);
		THROW_ON_FAILURE(hr);
		hr = UavHeap.Init(512, false);
		THROW_ON_FAILURE(hr);

		NAME_D3D12_OBJECT(RtvHeap.GetHeap(), L"RTV Heap Descriptor");
		NAME_D3D12_OBJECT(DsvHeap.GetHeap(), L"DSV Heap Descriptor");
		NAME_D3D12_OBJECT(SrvHeap.GetHeap(), L"SRV Heap Descriptor");
		NAME_D3D12_OBJECT(UavHeap.GetHeap(), L"UAV Heap Descriptor");


#ifdef CM_DEBUG
		ComPtr<ID3D12Debug>  DX12DebugController;
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&DX12DebugController));
#endif

		//Create the factory.
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&pDXGIFactory4));
		THROW_ON_FAILURE(hr);

		//Create the Device.
		hr = D3D12CreateDevice(nullptr, /* Use dedicated GPU.*/ D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));

		//If we were unable to create a Device using our
		//dedicated GPU, try to create one with the WARP
		//Device.
		if (FAILED(hr))
		{
			ComPtr<IDXGIAdapter> warpAdapter;
			HRESULT hr = pDXGIFactory4->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
			THROW_ON_FAILURE(hr);
			hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));
			THROW_ON_FAILURE(hr);
		}

		return hr;
	}

	void D3D12Context::CacheMSAAQuality()
	{

		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
		msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaQualityLevels.SampleCount = 4;
		msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msaaQualityLevels.NumQualityLevels = 0;

		HRESULT hr{ S_OK };

		hr = pDevice->CheckFeatureSupport
		(
			D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&msaaQualityLevels,
			sizeof(msaaQualityLevels)
		);
		THROW_ON_FAILURE(hr);

		MsaaQaulity = msaaQualityLevels.NumQualityLevels;
		CORE_ASSERT(MsaaQaulity > 0 && "Unexpected MSAA quality level.", "Unexpected MSAA quality level.");
	}

	void D3D12Context::SetDeferredReleasesFlag()
	{
		DeferralFlags[FrameIndex] = 1;
	}

	void D3D12Context::DeferredRelease(IUnknown* resource)
	{
		const UINT32 frame = FrameIndex;
		std::lock_guard{ DeferralsMutex };

		DeferredReleases[frame].push_back(resource);
		SetDeferredReleasesFlag();
	}


	void __declspec(noinline) D3D12Context::ProcessDeferrals(UINT32 frame)
	{
		std::lock_guard{ DeferralsMutex };

		DeferralFlags[frame] = 0;

		RtvHeap.ProcessDeferredFree(frame);
		DsvHeap.ProcessDeferredFree(frame);
		SrvHeap.ProcessDeferredFree(frame);
		UavHeap.ProcessDeferredFree(frame);

		std::vector<IUnknown*>& resources{ DeferredReleases[frame] };
		if (!resources.empty())
		{
			for (auto& resource : resources)
			{
				Release(resource);
			}
			resources.clear();
		}
	}

	void D3D12Context::IncrementFrame()
	{

		HRESULT hr{ S_OK };
		FrameIndex = (FrameIndex + 1) % FRAMES_IN_FLIGHT;

		// Has the GPU finished processing the commands of the current frame resource
		// If not, wait until the GPU has completed commands up to this fence point.
		if (pFence->GetCompletedValue() < RenderFrames[FrameIndex].Fence)
		{
			const HANDLE pEvent = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			const HRESULT pEventState = pFence->SetEventOnCompletion(RenderFrames[FrameIndex].Fence, pEvent);
			THROW_ON_FAILURE(pEventState);
			WaitForSingleObject(pEvent, INFINITE);
			CloseHandle(pEvent);
		}

		// Process pending free operations
		if (DeferralFlags[FrameIndex] == 1)
		{
			ProcessDeferrals(FrameIndex);
		}

	}


	void D3D12Context::Init()
	{
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
		{
			DebugController->EnableDebugLayer();
		}

		const HRESULT hr{ InitD3D12Core() };
		if(hr != S_OK)
		{
			CORE_ERROR("Failed to instantiate core D3D objects...");
			THROW_ON_FAILURE(hr);
		}
		else
		{
			CORE_TRACE("Creating D3D command objects...");
			CreateCommandObjects();
			CreateSwapChain();
			LogAdapters();

#ifdef CM_DEBUG
			pQueue->SetName(L"Graphics Queue");
			pGCL->SetName(L"Graphics List");
#endif
		}
		
	}

	void D3D12Context::CreateCommandObjects()
	{
		HRESULT hr{ S_OK };

		//Create the fence
		hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
		THROW_ON_FAILURE(hr);

		//Create the command queue description
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		//Now we can create the command queue
		hr = pDevice->CreateCommandQueue
		(
			&queueDesc,
			IID_PPV_ARGS(&pQueue)
		);
		THROW_ON_FAILURE(hr);

		//Create the command allocator 
		hr = pDevice->CreateCommandAllocator
		(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(pCmdAlloc.GetAddressOf())
		);
		THROW_ON_FAILURE(hr);

		//Create the direct command queue
		hr = pDevice->CreateCommandList
		(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			pCmdAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(pGCL.GetAddressOf())
		);
		THROW_ON_FAILURE(hr);

		pGCL->Close();
	}

	inline void D3D12Context::CreateSwapChain()
	{
		pSwapChain.Reset();

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		swapChainDesc.BufferDesc.Width = SwapChainWidth;
		swapChainDesc.BufferDesc.Height = SwapChainHeight;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = MsaaState ? 4 : 1;
		swapChainDesc.SampleDesc.Quality = MsaaState ? (MsaaQaulity - 1) : 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
		swapChainDesc.OutputWindow = pWindowHandle;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		THROW_ON_FAILURE(pDXGIFactory4->CreateSwapChain(pQueue.Get(), &swapChainDesc, pSwapChain.GetAddressOf()));
	}

	void D3D12Context::ShutDown()
	{
		
	}

	void D3D12Context::SwapBuffers()
	{
		
	}

	void D3D12Context::OnResizeSwapChain(INT32 x, INT32 y)
	{
		CORE_TRACE("Resizing swap chain");

		// Flush the rest of the render queue
		FlushRenderQueue();

		// Cache new width and height.
		SwapChainWidth = x;
		SwapChainHeight = y;

		// Rebuild the swap chain.
		CreateSwapChain();

		// Flush again to ensure all commands have been executed.
		FlushRenderQueue();

	}

	void inline D3D12Context::FlushRenderQueue()
	{
		SyncCounter++;
		HRESULT hr{ S_OK };
		hr = pQueue->Signal(pFence.Get(), SyncCounter);
		THROW_ON_FAILURE(hr);

		if (pFence->GetCompletedValue() < SyncCounter)
		{
			const HANDLE pEvent = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			const HRESULT pEventState = pFence->SetEventOnCompletion(SyncCounter, pEvent);
			THROW_ON_FAILURE(pEventState);
			WaitForSingleObject(pEvent, INFINITE);
			CloseHandle(pEvent);
		}
	}

	void inline D3D12Context::ExecuteGraphicsCommandList() const
	{
		ID3D12CommandList* cmdsLists[] = { pGCL.Get() };
		pQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	}

	void inline D3D12Context::IncrementSync() 
	{
		++SyncCounter; 
	}

	UINT32 D3D12Context::GetMSAAQuality()
	{
		return MsaaQaulity;
	}

	BOOL D3D12Context::GetMSAAState()
	{
		return MsaaState;
	}

	HWND D3D12Context::GetHwnd() const
	{
		return pWindowHandle;
	}

	UINT64 D3D12Context::GetSyncCount() const
	{
		return SyncCounter;
	}

	D3D12DescriptorHeap* D3D12Context::GetRTVHeap()
	{
		return &RtvHeap;
	}

	D3D12DescriptorHeap* D3D12Context::GetDSVHeap()
	{
		return &DsvHeap;
	}

	D3D12DescriptorHeap* D3D12Context::GetSRVHeap()
	{
		return &SrvHeap;
	}

	D3D12DescriptorHeap* D3D12Context::GetUAVHeap()
	{
		return &UavHeap;
	}

	ID3D12Device8* D3D12Context::GetDevice() const
	{
		return pDevice.Get();
	}

	D3D12RenderFrame* D3D12Context::CurrentRenderFrame()
	{
		return &RenderFrames[FrameIndex];
	}

	ID3D12Fence* D3D12Context::GetFence() const
	{
		return pFence.Get();
	}

	UINT D3D12Context::GetCurrentFrameIndex() const
	{
		return FrameIndex;
	}

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> D3D12Context::GetStaticSamplers()
	{
		const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
			0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
			1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
			2, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
			3, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
			4, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
			0.0f,                             // mipLODBias
			8);                               // maxAnisotropy

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
			5, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
			0.0f,                              // mipLODBias
			8);                                // maxAnisotropy

		return
		{
			pointWrap, pointClamp,
			linearWrap, linearClamp,
			anisotropicWrap, anisotropicClamp
		};
	}



#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }

	void D3D12Context::LogAdapters()
	{
		UINT i = 0;
		IDXGIAdapter* adapter = nullptr;
		std::vector<IDXGIAdapter*> adapterList;
		while (pDXGIFactory4->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);

			std::wstring text = L"***Adapter: ";
			text += desc.Description;
			text += L"\n";

			OutputDebugString(text.c_str());

			adapterList.push_back(adapter);

			++i;
		}

		for (size_t i = 0; i < adapterList.size(); ++i)
		{
			LogAdapterOutputs(adapterList[i]);
			ReleaseCom(adapterList[i]);
		}
	}

	void D3D12Context::LogAdapterOutputs(IDXGIAdapter* adapter)
	{
		UINT i = 0;
		IDXGIOutput* output = nullptr;
		while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC desc;
			output->GetDesc(&desc);

			std::wstring text = L"***Output: ";
			text += desc.DeviceName;
			text += L"\n";
			OutputDebugString(text.c_str());

			LogOutputDisplayModes(output, DXGI_FORMAT_R8G8B8A8_UNORM);

			ReleaseCom(output);

			++i;
		}
	}

	void D3D12Context::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
	{
		UINT count = 0;
		UINT flags = 0;

		// Call with nullptr to get list count.
		output->GetDisplayModeList(format, flags, &count, nullptr);

		std::vector<DXGI_MODE_DESC> modeList(count);
		output->GetDisplayModeList(format, flags, &count, &modeList[0]);

		for (auto& x : modeList)
		{
			UINT n = x.RefreshRate.Numerator;
			UINT d = x.RefreshRate.Denominator;
			std::wstring text =
				L"Width = " + std::to_wstring(x.Width) + L" " +
				L"Height = " + std::to_wstring(x.Height) + L" " +
				L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
				L"\n";

			::OutputDebugString(text.c_str());
		}
	}


}
