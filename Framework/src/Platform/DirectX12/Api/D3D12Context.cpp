#include "D3D12Context.h"
#include "Platform/DirectX12/_D3D12ConstantExpressions/D3D12ConstantExpressions.h"

#include "Framework/Core/Log/Log.h"

#include <filesystem>
#include <shlobj.h>
#include <pix.h>

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
		// TODO: Error, no PIX installation found
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
		//if (CommandQueue != nullptr)
		//{
		//	FlushCommandQueue();
		//	CommandQueue->Release();
		//	CommandQueue = nullptr;
		//}
		//if (SwapChain != nullptr)
		//{
		//	SwapChain->Release();
		//	SwapChain = nullptr;
		//}
		//if(Device != nullptr)
		//{
		//	Device->Release();
		//	Device = nullptr;
		//}
		//
		
	}

	void D3D12Context::Init()
	{

		// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
		// This may happen if the application is launched through the PIX UI. 
		/*if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
		{
			LoadLibrary(GetLatestWinPixGpuCapturerPath_Cpp17().c_str());
		}*/

		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
		{
			DebugController->EnableDebugLayer();
		}

		CreateDevice();
		CheckMSAAQualityAndCache();
		LogAdapters();

		CreateCommandObjects();
		CreateSwapChain();
		CreateCbvSrvUavHeap(1, 0, 1, 1);
		BuildRootSignature();

		Device->SetName(L"GPU Device");
		CommandQueue->SetName(L"Graphics Queue");
		CmdList->SetName(L"Graphics List");
		
	}

	void D3D12Context::FlushCommandQueue()
	{
		GPU_TO_CPU_SYNC_COUNT++;

		const HRESULT signalResult = CommandQueue->Signal(Fence.Get(), GPU_TO_CPU_SYNC_COUNT);
		THROW_ON_FAILURE(signalResult);

		if (Fence->GetCompletedValue() < GPU_TO_CPU_SYNC_COUNT)
		{
			const HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			const HRESULT eventCompletion = Fence->SetEventOnCompletion(GPU_TO_CPU_SYNC_COUNT, eventHandle);
			THROW_ON_FAILURE(eventCompletion);

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}


	void D3D12Context::ExecuteGraphicsCommandList() const
	{
		ID3D12CommandList* cmdsLists[] = { CmdList.Get() };
		CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	}

	void D3D12Context::SignalGPU() const
	{
		CommandQueue->Signal(Fence.Get(), GPU_TO_CPU_SYNC_COUNT);
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

		CbvSrvUavDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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
			IID_PPV_ARGS(Allocator.GetAddressOf())
		);

		//Create the direct command queue
		HRESULT cmdListResult = Device->CreateCommandList
		(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			Allocator.Get(),
			nullptr,
			IID_PPV_ARGS(CmdList.GetAddressOf())
		);

		//Now close the list. When we first use the command list
		//we'll need to reset it, for this to happen, the list must
		//be in a closed state.
		CmdList->Close();

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


	bool D3D12Context::CreateCbvSrvUavHeap
	(
		UINT opaqueRenderItemCount,
		UINT transparentRenderItemCount,
		UINT voxelWorldResources,
		UINT frameResourceCount
	)
	{
		const UINT objCount = opaqueRenderItemCount;
		// Need a CBV descriptor for each object for each frame resource,
		// +1 for the perPass CBV for each frame resource.
		const UINT numDescriptors = (objCount + 1) * frameResourceCount;

		// Save an offset to the start of the pass CBVs.  These are the last N descriptors.
		PassConstantBufferViewOffset = objCount * frameResourceCount;

		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = numDescriptors;
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

	bool D3D12Context::BuildRootSignature()
	{

		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[3];

		slotRootParameter[0].InitAsConstantBufferView(0);
		slotRootParameter[1].InitAsConstantBufferView(1);
		slotRootParameter[2].InitAsConstantBufferView(2);

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc
		(
			3, 
			slotRootParameter, 
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature
		(
			&rootSigDesc, 
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		THROW_ON_FAILURE(hr);

		THROW_ON_FAILURE
		(
			Device->CreateRootSignature
			(
				0,
				serializedRootSig->GetBufferPointer(),
				serializedRootSig->GetBufferSize(),
				IID_PPV_ARGS(&RootSignature)
			)
		);

		return true;

	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12Context::GetConstantBufferViewCpu() const
	{
		return CbvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12Context::GetConstantBufferViewGpu() const
	{
		return CbvHeap->GetGPUDescriptorHandleForHeapStart();
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