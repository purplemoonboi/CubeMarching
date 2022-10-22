#include "Framework/cmpch.h"
#include "DX12Renderer.h"

#include "Framework/Core/Log/Log.h"

namespace DX12Framework
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

	}

	void DX12Renderer::Resize(INT32 bufferWidth, INT32 bufferHeight)
	{
		CORE_ASSERT(Device, "Device failed...");
		CORE_ASSERT(SwapChain, "Swap chain encountered an error...");
		CORE_ASSERT(DirectCommandListAllocator, "Command alloc com failed...");


		FlushCommandQueue();

		CommandList->Reset(DirectCommandListAllocator.Get(), nullptr);

		for(UINT i = 0; i < SwapChainBufferCount; ++i)
		{
			SwapChainBuffer[i].Reset();
		}

		DepthStencilBuffer.Reset();

		HRESULT resizeSCResult = SwapChain->ResizeBuffers
		(
			SwapChainBufferCount,
			ViewportWidth,
			ViewportHeight,
			BackBufferFormat,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		);

		CurrentBackBuffer = 0;

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
		for(UINT i = 0; i < SwapChainBufferCount; ++i)
		{
			HRESULT scbResult = SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffer[i]));
			Device->CreateRenderTargetView(SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
			rtvHeapHandle.Offset(1, RtvDescriptorSize);
		}

		CreateDepthStencilResource();

		auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition
		(
			DepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		);

		CommandList->ResourceBarrier
		(
			1,
			&resourceBarrier
		);

		HRESULT cmdListResult = CommandList->Close();

		FlushCommandQueue();

		ScreenViewport.TopLeftX = 0;
		ScreenViewport.TopLeftY = 0;
		ScreenViewport.Width = static_cast<float>(ViewportWidth);
		ScreenViewport.Height = static_cast<float>(ViewportHeight);
		ScreenViewport.MinDepth = 0.0f;
		ScreenViewport.MaxDepth = 1.0f;

		ScissorRect = { 0, 0, ViewportWidth, ViewportHeight };
	}

	void DX12Renderer::FlushCommandQueue()
	{
		FenceSyncCount++;

		HRESULT cmdQueueResult = CommandQueue->Signal(SyncFence.Get(), FenceSyncCount++);

		if (SyncFence->GetCompletedValue() < FenceSyncCount)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	bool DX12Renderer::CreateDepthStencilResource()
	{
		/* Create the depth stencil description*/
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = ViewportWidth;
		depthStencilDesc.Height = ViewportHeight;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthStencilDesc.SampleDesc.Count = MSAA4xQaulity;
		depthStencilDesc.SampleDesc.Quality = MSAA4xQaulity ? (MSAA4xQaulity - 1) : 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DepthStencilFormat;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		//taking the address of r-value is non-standard... implemented local
		//variable for l-value
		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		HRESULT resourceResult = Device->CreateCommittedResource
		(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf())
		);


		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DepthStencilFormat;
		dsvDesc.Texture2D.MipSlice = 0;
		Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());


		return true;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DX12Renderer::DepthStencilView() const
	{
		return DsvHeap->GetCPUDescriptorHandleForHeapStart();
	}
}