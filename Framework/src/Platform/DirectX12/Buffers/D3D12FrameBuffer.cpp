#include "Framework/cmpch.h"
#include "D3D12FrameBuffer.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Framework/Core/Log/Log.h"

namespace Engine
{
	D3D12FrameBuffer::D3D12FrameBuffer(const D3D12FrameBuffer& other)
		:
		RtvHeap(nullptr),
		DsvHeap(nullptr),
		BackBufferIndex(0),
		RtvDescriptorSize(0),
		DsvDescriptorSize(0)
	{
		FrameBufferSpecs = other.FrameBufferSpecs;
		ScissorRect = other.ScissorRect;
		ScreenViewport = other.ScreenViewport;
	}

	D3D12FrameBuffer::D3D12FrameBuffer(const FrameBufferSpecifications& fBufferSpecs)
		:
		FrameBufferSpecs(fBufferSpecs),
		RtvHeap(nullptr),
		DsvHeap(nullptr),
		BackBufferIndex(0),
		RtvDescriptorSize(0),
		DsvDescriptorSize(0)
	{
		
		ScissorRect = {};
		ScreenViewport = {};
	}

	D3D12FrameBuffer::~D3D12FrameBuffer()
	{
		RtvHeap->Release();
		RtvHeap = nullptr;
		DsvHeap->Release();
		DsvHeap = nullptr;
	}

	void D3D12FrameBuffer::Init(GraphicsContext* context)
	{
		Context = dynamic_cast<D3D12Context*>(context);

		DsvDescriptorSize = Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		RtvDescriptorSize = Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// Resource 
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;

		THROW_ON_FAILURE(Context->Device->CreateDescriptorHeap
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

		THROW_ON_FAILURE(Context->Device->CreateDescriptorHeap
		(
			&dsvHeapDesc,
			IID_PPV_ARGS(DsvHeap.GetAddressOf())
		));


		//Create the committed resource
		const HRESULT resourceResult = Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(BackBufferFormat, 1920, 1080),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&Resource)
		);
		THROW_ON_FAILURE(resourceResult);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = BackBufferFormat;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		FrameBufferSrv = D3D12Utils::CreateShaderResourceView(srvDesc, Resource.Get(), FrameBufferSrvCpu);

	}

	void D3D12FrameBuffer::Bind()
	{
		CORE_ASSERT(Context->Device, "The 'D3D device' has failed...");
		CORE_ASSERT(Context->SwapChain, "The 'swap chain' has failed...");
		CORE_ASSERT(Context->GraphicsCmdList, "The 'graphics command list' has failed...");

		// Indicate there will be a transition made to the resource.
		Context->GraphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));

		Context->GraphicsCmdList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(RtvHeap->GetCPUDescriptorHandleForHeapStart(),
				BackBufferIndex,
				RtvDescriptorSize
			),
			DirectX::Colors::SandyBrown,
			0,
			nullptr
		);

		Context->GraphicsCmdList->ClearDepthStencilView(DsvHeap->GetCPUDescriptorHandleForHeapStart(),
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			1.0f, 0, 0, nullptr
		);

		Context->GraphicsCmdList->RSSetViewports(1, &ScreenViewport);
		Context->GraphicsCmdList->RSSetScissorRects(1, &ScissorRect);

		Context->GraphicsCmdList->OMSetRenderTargets(1, 
			&CD3DX12_CPU_DESCRIPTOR_HANDLE(RtvHeap->GetCPUDescriptorHandleForHeapStart(), BackBufferIndex, RtvDescriptorSize),
			true, &DsvHeap->GetCPUDescriptorHandleForHeapStart()
		);

	}

	void D3D12FrameBuffer::UnBind()
	{
		D3D12_TEXTURE_COPY_LOCATION copySrc = {};
		copySrc.pResource = SwapChainBuffer[BackBufferIndex].Get();
		copySrc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		copySrc.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION copyDst = {};
		copyDst.pResource = Resource.Get();
		copyDst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		copyDst.SubresourceIndex = 0;

		Context->GraphicsCmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(SwapChainBuffer[BackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE)
		);

		Context->GraphicsCmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(Resource.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST)
		);

		Context->GraphicsCmdList->CopyTextureRegion(&copyDst, 0, 0, 0, &copySrc, nullptr);

		Context->GraphicsCmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(SwapChainBuffer[BackBufferIndex].Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET)
		);

		Context->GraphicsCmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ)
		);


		Context->GraphicsCmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				SwapChainBuffer[BackBufferIndex].Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT
			)
		);
	}

	void D3D12FrameBuffer::RebuildFrameBuffer(FrameBufferSpecifications& specifications)
	{
		CORE_ASSERT(Context->Device, "The 'D3D device' has failed...");
		CORE_ASSERT(Context->SwapChain, "The 'swap chain' has failed...");
		CORE_ASSERT(Context->GraphicsCmdList, "The 'graphics command list' has failed...");

		FrameBufferSpecs = specifications;

		// Flush before changing any resources.
		Context->FlushCommandQueue();

		const HRESULT resetResult = Context->GraphicsCmdList->Reset(Context->Allocator.Get(), nullptr);
		THROW_ON_FAILURE(resetResult);

		// Release the previous resources we will be recreating.
		for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		{
			SwapChainBuffer[i].Reset();
		}

		Context->DepthStencilBuffer.Reset();


		// Resize the swap chain.
		const HRESULT resizeResult = Context->SwapChain->ResizeBuffers
		(
			SWAP_CHAIN_BUFFER_COUNT,
			FrameBufferSpecs.Width, FrameBufferSpecs.Height,
			BackBufferFormat,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		);
		THROW_ON_FAILURE(resizeResult);


		BackBufferIndex = 0;

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_RENDER_TARGET_VIEW_DESC desc = {};
		

		for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
		{
			Context->SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffer[i]));

			Context->Device->CreateRenderTargetView
			(
				SwapChainBuffer[i].Get(),
				nullptr,
				rtvHeapHandle
			);
			SwapChainBuffer->Get()->SetName(L"Swap Chain");
			THROW_ON_FAILURE(Context->Device->GetDeviceRemovedReason());
			rtvHeapHandle.Offset(1, RtvDescriptorSize);
		}

		// Create the depth/stencil buffer and view.
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = FrameBufferSpecs.Width;
		depthStencilDesc.Height = FrameBufferSpecs.Height;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;

		// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
		// the depth buffer.  Therefore, because we need to create two views to the same resource:
		//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
		//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
		// we need to create the depth buffer resource with a typeless format.  
		depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthStencilDesc.SampleDesc.Count = Context->GetMsaaState() ? 4 : 1;
		depthStencilDesc.SampleDesc.Quality = Context->GetMsaaState() ? (Context->GetMsaaQaulity() - 1) : 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DepthStencilFormat;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		const HRESULT resourceResult = Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(Context->DepthStencilBuffer.GetAddressOf())
		);
		THROW_ON_FAILURE(resourceResult);

		THROW_ON_FAILURE(Context->Device->GetDeviceRemovedReason());

		// Create descriptor to mip level 0 of entire resource using the format of the resource.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;

		Context->Device->CreateDepthStencilView
		(
			Context->DepthStencilBuffer.Get(),
			&dsvDesc,
			GetDepthStencilViewCpu()
		);


		// Transition the resource from its initial state to be used as a depth buffer.
		Context->GraphicsCmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				Context->DepthStencilBuffer.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_DEPTH_WRITE
			)
		);

		//Create the committed resource
		const HRESULT hr = Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(BackBufferFormat, specifications.Width, 
				specifications.Height),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&Resource)
		);
		THROW_ON_FAILURE(hr);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = BackBufferFormat;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		Context->Device->CreateShaderResourceView(Resource.Get(), &srvDesc, FrameBufferSrvCpu);
		const HRESULT deviceRemovedReason = Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);

		// Execute the resize commands.
		const HRESULT closeResult = Context->GraphicsCmdList->Close();
		THROW_ON_FAILURE(closeResult);

		ID3D12CommandList* cmdsLists[] = { Context->GraphicsCmdList.Get() };
		Context->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// Wait until resize is complete.
		Context->FlushCommandQueue();

		// Update the viewport transform to cover the client area.
		ScreenViewport.TopLeftX = FrameBufferSpecs.OffsetX;
		ScreenViewport.TopLeftY = FrameBufferSpecs.OffsetY;
		ScreenViewport.Width = static_cast<float>(FrameBufferSpecs.Width);
		ScreenViewport.Height = static_cast<float>(FrameBufferSpecs.Height);
		ScreenViewport.MinDepth = 0.0f;
		ScreenViewport.MaxDepth = 1.0f;

		ScissorRect = { 0, 0, FrameBufferSpecs.Width, FrameBufferSpecs.Height };
	}

	void D3D12FrameBuffer::SetBufferSpecifications(FrameBufferSpecifications& fbSpecs)
	{
		FrameBufferSpecs.Width = fbSpecs.Width;
		FrameBufferSpecs.Height = fbSpecs.Height;
		FrameBufferSpecs.OffsetX = fbSpecs.OffsetX;
		FrameBufferSpecs.OffsetY = fbSpecs.OffsetY;

	}

	UINT64 D3D12FrameBuffer::GetFrameBuffer() const
	{
		return FrameBufferSrv.ptr;
	}

	ID3D12Resource* D3D12FrameBuffer::CurrentBackBuffer() const
	{
		return SwapChainBuffer[BackBufferIndex].Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12FrameBuffer::GetCurrentBackBufferViewCpu() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE
		(
			RtvHeap->GetCPUDescriptorHandleForHeapStart(),
			BackBufferIndex,
			RtvDescriptorSize
		);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12FrameBuffer::GetDepthStencilViewCpu() const
	{
		return DsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	INT32 D3D12FrameBuffer::GetWidth() const
	{
		return FrameBufferSpecs.Width;
	}

	INT32 D3D12FrameBuffer::GetHeight() const
	{
		return FrameBufferSpecs.Height;
	}
}
