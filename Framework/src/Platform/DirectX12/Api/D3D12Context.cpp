#include "D3D12Context.h"
#include "Platform/DirectX12/_D3D12ConstantExpressions/D3D12ConstantExpressions.h"

#include "Framework/Core/Log/Log.h"

#include <filesystem>
#include <shlobj.h>



namespace Foundation
{


	D3D12Context::D3D12Context
	(
		HWND windowHandle,
		INT32 swapChainBufferWidth,
		INT32 swapChainBufferHeight
	)
		:
		pWindowHandle(windowHandle),
		SwapChainWidth(swapChainBufferWidth),
		SwapChainHeight(swapChainBufferHeight)
	{
		CORE_ASSERT(pWindowHandle, "Window handle is null!");
		D3D12Context::Init();
	}

	D3D12Context::D3D12Context(const D3D12Context& other)
	{}


	D3D12Context::~D3D12Context()
	{
	
		if (pSwapChain != nullptr)
		{
			pSwapChain.Reset();
			pSwapChain = nullptr;
		}

		if(pDevice != nullptr)
		{
			pDevice.Reset();
			pDevice = nullptr;
		}
		
	}

	void D3D12Context::Init()
	{


		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
		{
			DebugController->EnableDebugLayer();
		}
		
		
		CreateDevice();
		CheckMSAAQualityAndCache();
		LogAdapters();
		CreateCommandObjects();
		CreateSwapChain();

		pDevice->SetName(L"GPU Device");
		pQueue->SetName(L"Graphics Queue");
		pGCL->SetName(L"Graphics List");
		
	}

	void D3D12Context::FlushCommandQueue()
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


	void D3D12Context::ExecuteGraphicsCommandList() const
	{
		ID3D12CommandList* cmdsLists[] = { pGCL.Get() };
		pQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	}

	void D3D12Context::SignalGPU() const
	{
		pQueue->Signal(pFence.Get(), SyncCounter);
	}

	void D3D12Context::CreateDevice()
	{
		HRESULT hr{ S_OK };

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
			HRESULT warpEnumResult = pDXGIFactory4->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
			HRESULT warpDeviceResult = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));
		}

		//Create the fence
		hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
		THROW_ON_FAILURE(hr);
	}


	void D3D12Context::CreateCommandObjects()
	{
		//Create the command queue description
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		//Now we can create the command queue
		HRESULT qDescResult = pDevice->CreateCommandQueue
		(
			&queueDesc,
			IID_PPV_ARGS(&pQueue)
		);

		//Create the command allocator 
		HRESULT cmdQueueAllocResult = pDevice->CreateCommandAllocator
		(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(pCmdAlloc.GetAddressOf())
		);

		//Create the direct command queue
		HRESULT cmdListResult = pDevice->CreateCommandList
		(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			pCmdAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(pGCL.GetAddressOf())
		);

		//Now close the list. When we first use the command list
		//we'll need to reset it, for this to happen, the list must
		//be in a closed state.
		pGCL->Close();

	}

	void D3D12Context::CheckMSAAQualityAndCache()
	{
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
		msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaQualityLevels.SampleCount = 4;
		msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msaaQualityLevels.NumQualityLevels = 0;

		pDevice->CheckFeatureSupport
		(
			D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&msaaQualityLevels,
			sizeof(msaaQualityLevels)
		);

		MsaaQaulity = msaaQualityLevels.NumQualityLevels;
		CORE_ASSERT(MsaaQaulity > 0 && "Unexpected MSAA quality level.", "Unexpected MSAA quality level.");

	}

	void D3D12Context::CreateSwapChain()
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