#include "D3D12Context.h"
#include "Platform/DirectX12/_D3D12ConstantExpressions/D3D12ConstantExpressions.h"

#include "Framework/Core/Log/Log.h"

#include <filesystem>
#include <shlobj.h>

#ifdef USE_PIX
#include <pix3.h>
#endif

static std::wstring GetLatestWinPixGpuCapturerPath_Cpp17()
{
	LPWSTR programFilesPath = nullptr;
	SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

	std::filesystem::path pixInstallationPath = programFilesPath;
	pixInstallationPath /= "Microsoft PIX";

	std::wstring newestVersionFound;

	for (auto const& directory_entry : std::filesystem::directory_iterator(pixInstallationPath))
	{
		if (directory_entry.is_directory())
		{
			if (newestVersionFound.empty() || newestVersionFound < directory_entry.path().filename().c_str())
			{
				newestVersionFound = directory_entry.path().filename().c_str();
			}
		}
	}

	if (newestVersionFound.empty())
	{
		return L"EMPTY";
	}

	return pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll";
}


namespace Engine
{


	D3D12Context::D3D12Context
	(
		HWND windowHandle,
		INT32 swapChainBufferWidth,
		INT32 swapChainBufferHeight
	)
		:
		WindowHandle(windowHandle),
		SwapChainBufferWidth(swapChainBufferWidth),
		SwapChainBufferHeight(swapChainBufferHeight)
	{
		CORE_ASSERT(WindowHandle, "Window handle is null!");
		D3D12Context::Init();
	}

	D3D12Context::D3D12Context(const D3D12Context& other)
	{}


	D3D12Context::~D3D12Context()
	{
		if (CommandQueue != nullptr)
		{
			CommandQueue.Reset();
			CommandQueue = nullptr;
		}
		if (SwapChain != nullptr)
		{
			SwapChain.Reset();
			SwapChain = nullptr;
		}
		if(Device != nullptr)
		{
			Device.Reset();
			Device = nullptr;
		}
		
		
	}

	void D3D12Context::Init()
	{
#ifdef USE_PIX
		// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
		// This may happen if the application is launched through the PIX UI. 
		if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
		{
			CORE_TRACE("Injecting WinPixCapturer.dll...")
			const wchar_t* path = GetLatestWinPixGpuCapturerPath_Cpp17().c_str();
			if(path != L"EMPTY")
			{
				LoadLibrary(path);
				GpuCaptureLib = PIXLoadLatestWinPixGpuCapturerLibrary();
				GpuTimingLib  = PIXLoadLatestWinPixTimingCapturerLibrary();
			}
			else
			{
				CORE_WARNING("PIX Capturer Invalid!");
			}
		}
		else
		{
			CORE_WARNING("Failed to inject WinPixCapturer.dll")
		}
#endif

		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
		{
			DebugController->EnableDebugLayer();
		}
		
		
		CreateDevice();
		CheckMSAAQualityAndCache();
		LogAdapters();
		CreateCommandObjects();
		CreateSwapChain();
		CreateRtvAndDsvHeaps();
		CreateRootSignature();

		Device->SetName(L"GPU Device");
		CommandQueue->SetName(L"Graphics Queue");
		ResourceCommandList->SetName(L"Graphics List");
		
	}

	void D3D12Context::FlushCommandQueue()
	{
		SyncCounter++;

		const HRESULT signalResult = CommandQueue->Signal(Fence.Get(), SyncCounter);
		THROW_ON_FAILURE(signalResult);

		if (Fence->GetCompletedValue() < SyncCounter)
		{
			const HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			const HRESULT eventCompletion = Fence->SetEventOnCompletion(SyncCounter, eventHandle);
			THROW_ON_FAILURE(eventCompletion);

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}


	void D3D12Context::ExecuteGraphicsCommandList() const
	{
		ID3D12CommandList* cmdsLists[] = { ResourceCommandList.Get() };
		CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	}

	void D3D12Context::SignalGPU() const
	{
		CommandQueue->Signal(Fence.Get(), SyncCounter);
	}

	bool D3D12Context::CreateDevice()
	{
#ifdef CM_DEBUG
		ComPtr<ID3D12Debug>  DX12DebugController;
		HRESULT debugResult = D3D12GetDebugInterface(IID_PPV_ARGS(&DX12DebugController));
#endif


		//Create the factory.
		HRESULT dxgiFactoryResult = CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory));


		//Create the Device.
		HRESULT hardwareResult = D3D12CreateDevice
		(
			nullptr,//Use dedicated GPU.
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&Device)
		);


		//If we were unable to create a Device using our
		//dedicated GPU, try to create one with the WARP
		//Device.
		if (FAILED(hardwareResult))
		{
			ComPtr<IDXGIAdapter> warpAdapter;
			HRESULT warpEnumResult = DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

			HRESULT warpDeviceResult = D3D12CreateDevice
			(
				warpAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&Device)
			);

		}

		//Create the fence
		THROW_ON_FAILURE(Device->CreateFence
		(
			0,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&Fence)
		));


		return true;
	}


	bool D3D12Context::CreateCommandObjects()
	{
		//Create the command queue description
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		//Now we can create the command queue
		HRESULT qDescResult = Device->CreateCommandQueue
		(
			&queueDesc,
			IID_PPV_ARGS(&CommandQueue)
		);

		//Create the command allocator 
		HRESULT cmdQueueAllocResult = Device->CreateCommandAllocator
		(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(ResourceAlloc.GetAddressOf())
		);

		//Create the direct command queue
		HRESULT cmdListResult = Device->CreateCommandList
		(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			ResourceAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(ResourceCommandList.GetAddressOf())
		);

		//Now close the list. When we first use the command list
		//we'll need to reset it, for this to happen, the list must
		//be in a closed state.
		ResourceCommandList->Close();

		return true;
	}

	bool D3D12Context::CheckMSAAQualityAndCache()
	{
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
		msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaQualityLevels.SampleCount = 4;
		msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msaaQualityLevels.NumQualityLevels = 0;

		Device->CheckFeatureSupport
		(
			D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&msaaQualityLevels,
			sizeof(msaaQualityLevels)
		);

		MsaaQaulity = msaaQualityLevels.NumQualityLevels;
		CORE_ASSERT(MsaaQaulity > 0 && "Unexpected MSAA quality level.", "Unexpected MSAA quality level.");

		return true;
	}

	bool D3D12Context::CreateRtvAndDsvHeaps()
	{
		// Resource 
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT + 1;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;

		THROW_ON_FAILURE(Device->CreateDescriptorHeap
		(
			&rtvHeapDesc,
			IID_PPV_ARGS(RtvHeap.GetAddressOf())
		));

		// Depth stencil
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;

		THROW_ON_FAILURE(Device->CreateDescriptorHeap
		(
			&dsvHeapDesc,
			IID_PPV_ARGS(DsvHeap.GetAddressOf())
		));

		return true;
	}

	


	bool D3D12Context::CreateSwapChain()
	{
		SwapChain.Reset();

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		swapChainDesc.BufferDesc.Width = SwapChainBufferWidth;
		swapChainDesc.BufferDesc.Height = SwapChainBufferHeight;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = MsaaState ? 4 : 1;
		swapChainDesc.SampleDesc.Quality = MsaaState ? (MsaaQaulity - 1) : 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
		swapChainDesc.OutputWindow = WindowHandle;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		THROW_ON_FAILURE(DXGIFactory->CreateSwapChain
		(
			CommandQueue.Get(),
			&swapChainDesc,
			SwapChain.GetAddressOf()
		));


		return true;
	}




#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }

	void D3D12Context::LogAdapters()
	{
		UINT i = 0;
		IDXGIAdapter* adapter = nullptr;
		std::vector<IDXGIAdapter*> adapterList;
		while (DXGIFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
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