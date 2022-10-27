#include "DX12GraphicsContext.h"

#include "Framework/Core/Log/Log.h"

namespace DX12Framework
{
	DX12GraphicsContext::DX12GraphicsContext
	(
		HWND windowHandle,
		INT32 viewportWidth,
		INT32 viewportHeight
	)
		:
		WindowHandle(windowHandle),
		ViewportWidth(viewportWidth),
		ViewportHeight(viewportHeight)
	{
		CORE_ASSERT(WindowHandle, "Window handle is null!");
		Init();
	}

	DX12GraphicsContext::~DX12GraphicsContext()
	{
	}

	void DX12GraphicsContext::Init()
	{

		CreateDevice();
		CheckMSAAQualityAndCache();
		LogAdapters();

		CreateCommandObjects();
		CreateSwapChain();
		CreateDsvAndRtvDescriptorHeaps();

		CreateCbvSrvUavDescriptorHeap();
	}

	void DX12GraphicsContext::FlushCommandQueue()
	{
		//TODO: This isn't very efficient
		GPU_TO_CPU_SYNC_COUNT++;


		THROW_ON_FAILURE(CommandQueue->Signal(Fence.Get(), GPU_TO_CPU_SYNC_COUNT));

		auto a = Fence->GetCompletedValue();
		if (a < GPU_TO_CPU_SYNC_COUNT)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

			THROW_ON_FAILURE(Fence->SetEventOnCompletion(GPU_TO_CPU_SYNC_COUNT, eventHandle));

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}

	}

	bool DX12GraphicsContext::CreateDevice()
	{
#ifdef CM_DEBUG
		Microsoft::WRL::ComPtr<ID3D12Debug>  DX12DebugController;
		HRESULT debugResult = D3D12GetDebugInterface(IID_PPV_ARGS(&DX12DebugController));
#endif


		//Create the factory.
		HRESULT dxgiFactoryResult = CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory));


		//Create the Device.
		HRESULT hardwareResult = D3D12CreateDevice
		(
			nullptr,//Means we'd like to use dedicated GPU.
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&Device)
		);


		//If we were unable to create a Device using our
		//dedicated GPU, try to create one with the WARP
		//Device.
		if (FAILED(hardwareResult))
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
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

		DsvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CbvSrvUavDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);




		return true;
	}

	bool DX12GraphicsContext::CreateCommandObjects()
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
			IID_PPV_ARGS(DirCmdListAlloc.GetAddressOf())
		);

		//Create the direct command queue
		HRESULT cmdListResult = Device->CreateCommandList
		(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			DirCmdListAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(GraphicsCmdList.GetAddressOf())
		);

		//Now close the list. When we first use the command list
		//we'll need to reset it, for this to happen, the list must
		//be in a closed state.
		GraphicsCmdList->Close();

		return true;
	}

	bool DX12GraphicsContext::CheckMSAAQualityAndCache()
	{
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
		msaaQualityLevels.Format = BackBufferFormat;
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

	bool DX12GraphicsContext::CreateDsvRtvAndCbvDescriptorHeaps()
	{
		// Resource 
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
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


		// Const buffer
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = 1U;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;

		THROW_ON_FAILURE
		(
			Device->CreateDescriptorHeap
			(
				&cbvHeapDesc, 
				IID_PPV_ARGS(&CbvHeap)
			)
		);

		return true;
	}


	bool DX12GraphicsContext::CreateSwapChain()
	{
		SwapChain.Reset();

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		swapChainDesc.BufferDesc.Width = ViewportWidth;
		swapChainDesc.BufferDesc.Height = ViewportHeight;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format = BackBufferFormat;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = MsaaState ? 4 : 1;
		swapChainDesc.SampleDesc.Quality = MsaaState ? (MsaaQaulity - 1) : 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = SwapChainBufferCount;
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



	ID3D12Resource* DX12GraphicsContext::CurrentBackBuffer() const
	{
		return SwapChainBuffer[BackBufferIndex].Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DX12GraphicsContext::CurrentBackBufferView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE
		(
			RtvHeap->GetCPUDescriptorHandleForHeapStart(),
			BackBufferIndex,
			RtvDescriptorSize
		);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DX12GraphicsContext::DepthStencilView() const
	{
		return DsvHeap->GetCPUDescriptorHandleForHeapStart();
	}




#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }

	void DX12GraphicsContext::LogAdapters()
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

	void DX12GraphicsContext::LogAdapterOutputs(IDXGIAdapter* adapter)
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

			LogOutputDisplayModes(output, BackBufferFormat);

			ReleaseCom(output);

			++i;
		}
	}

	void DX12GraphicsContext::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
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