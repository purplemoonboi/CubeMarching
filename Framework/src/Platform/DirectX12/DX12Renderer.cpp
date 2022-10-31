#include "Framework/cmpch.h"
#include "DX12Renderer.h"

#include "Framework/Core/Log/Log.h"

namespace Engine
{
	DX12Renderer::DX12Renderer(HWND	windowHandle, INT32 bufferWidth, INT32 bufferHeight)
		:
		RtvDescriptorSize(0),
		DsvDescriptorSize(0),
		CbvSrvUavDescriptorSize(0),
		D3DDriverType(D3D_DRIVER_TYPE_HARDWARE),
		BackBufferFormat(DXGI_FORMAT_R8G8B8A8_UNORM),
		DepthStencilFormat(DXGI_FORMAT_D24_UNORM_S8_UINT),
		FenceSyncCount(0),
		ViewportWidth(bufferWidth),
		ViewportHeight(bufferHeight),
		ApplicationWindowHandle(&windowHandle)
	{
		CreateDevice();
		CheckMSAAQualityAndCache();
		CreateCommandObjects();
		CreateSwapChain();
		CreateRtvDescriptorHeap();
		CreateDsvDescriptorHeap();
		CreateCbvSrvUavDescriptorHeap();
	}

	DX12Renderer::~DX12Renderer()
	{

	}

	RefPointer<DX12Renderer> DX12Renderer::Create(HWND windowHandle, INT32 bufferWidth, INT32 bufferHeight)
	{
		return CreateRef<DX12Renderer>(windowHandle, bufferWidth, bufferHeight);
	}

	bool DX12Renderer::CreateDevice()
	{
#ifdef CM_DEBUG
		Microsoft::WRL::ComPtr<ID3D12Debug>  DX12DebugController;
		HRESULT debugResult = D3D12GetDebugInterface(IID_PPV_ARGS(&DX12DebugController));
#endif


		//Create the factory.
		HRESULT dxgiFactoryResult = CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory));


		//Create the device.
		HRESULT hardwareResult = D3D12CreateDevice
		(
			nullptr,//Means we'd like to use dedicated GPU.
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&Device)
		);


		//If we were unable to create a device using our
		//dedicated GPU, try to create one with the WARP
		//device.
		if(FAILED(hardwareResult))
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
		HRESULT fenceResult = Device->CreateFence
		(
			0, 
			D3D12_FENCE_FLAG_NONE, 
			IID_PPV_ARGS(&SyncFence)
		);
		return true;
	}

	bool DX12Renderer::CheckMSAAQualityAndCache()
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

		MSAA4xQaulity = msaaQualityLevels.NumQualityLevels;
		//CORE_ASSERT("Unexpected MSAA quality level.", (MSAA4xQaulity > 0), );

		return true;
	}

	bool DX12Renderer::CreateCommandObjects()
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
			IID_PPV_ARGS(DirectCommandListAllocator.GetAddressOf())
		);

		//Create the direct command queue
		HRESULT cmdListResult = Device->CreateCommandList
		(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			DirectCommandListAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(CommandList.GetAddressOf())
		);

		//Now close the list. When we first use the command list
		//we'll need to reset it, for this to happen, the list must
		//be in a closed state.
		CommandList->Close();

		return true;
	}

	bool DX12Renderer::CreateSwapChain()
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
		swapChainDesc.SampleDesc.Count = MSAA4xQaulity;
		swapChainDesc.SampleDesc.Quality = MSAA4xQaulity ? (MSAA4xQaulity - 1) : 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = SwapChainBufferCount;
		swapChainDesc.OutputWindow = *ApplicationWindowHandle;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		DXGIFactory->CreateSwapChain
		(
			CommandQueue.Get(),
			&swapChainDesc,
			SwapChain.GetAddressOf()
		);


		return true;
	}

	bool DX12Renderer::CreateRtvDescriptorHeap()
	{
		RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;

		HRESULT rtvHeapResult = Device->CreateDescriptorHeap
		(
			&rtvHeapDesc,
			IID_PPV_ARGS(RtvHeap.GetAddressOf())
		);

		return true;
	}

	bool DX12Renderer::CreateDsvDescriptorHeap()
	{
		DsvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type		   = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags		   = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask	   = 0;

		HRESULT dsvHeapResult = Device->CreateDescriptorHeap
		(
			&dsvHeapDesc,
			IID_PPV_ARGS(DsvHeap.GetAddressOf())
		);

		return true;
	}

	bool DX12Renderer::CreateCbvSrvUavDescriptorHeap()
	{
		//TODO: Finish implementing this!
		CbvSrvUavDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		return true;
	}

}
