#include "D3D12Utilities.h"
#include "Framework/Core/Core.h"
#include "Framework/Core/Log/Log.h"

#include "Platform/DirectX12/D3D12/D3D12.h"
#include "Platform/DirectX12/Heap/D3D12HeapManager.h"

namespace Foundation::Graphics::D3D12
{

	__inline D3D12DescriptorHandle CreateRenderTargetView(
		const D3D12_RENDER_TARGET_VIEW_DESC* desc,
		ID3D12Resource* resource
	) 
	{
		auto* pRtvHeap = gD3D12Context->GetRTVHeap();
		auto* pDevice = gD3D12Context->GetDevice();
		const auto handle = pRtvHeap->Allocate();

		pDevice->CreateRenderTargetView(resource, nullptr, handle.CpuHandle);
		return handle;
	}

	__inline HRESULT RefreshRenderTargetView(
		const D3D12_RENDER_TARGET_VIEW_DESC* desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource
	)
	{
		auto* pDevice = gD3D12Context->GetDevice();
		pDevice->CreateRenderTargetView(resource, desc, handle.CpuHandle);
		const HRESULT hr = pDevice->GetDeviceRemovedReason();
		return hr;
	}

	__inline D3D12DescriptorHandle CreateDepthStencilView(
		const D3D12_DEPTH_STENCIL_VIEW_DESC* desc, 
		ID3D12Resource* resource
	)
	{
		auto* pDsvHeap = gD3D12Context->GetDSVHeap();
		auto* pDevice = gD3D12Context->GetDevice();
		const auto handle = pDsvHeap->Allocate();

		pDevice->CreateDepthStencilView(resource, desc, handle.CpuHandle);
		return handle;
	}

	HRESULT RefreshDepthStencilView(
		const D3D12_DEPTH_STENCIL_VIEW_DESC* desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource)
	{
		auto* pDevice = gD3D12Context->GetDevice();
		pDevice->CreateDepthStencilView(resource, desc, handle.CpuHandle);
		const HRESULT hr = pDevice->GetDeviceRemovedReason();
		return hr;
	}

	__inline D3D12DescriptorHandle CreateShaderResourceView(
		const D3D12_SHADER_RESOURCE_VIEW_DESC& desc,
		ID3D12Resource* resource
	)
	{
		auto* pSrvHeap = gD3D12Context->GetSRVHeap();
		auto* pDevice = gD3D12Context->GetDevice();

		const auto handle = pSrvHeap->Allocate();
		pDevice->CreateShaderResourceView(resource, &desc, handle.CpuHandle);
		return handle;
	}

	__inline HRESULT RefreshShaderResourceViews(
		const D3D12_SHADER_RESOURCE_VIEW_DESC* desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource
	)
	{
		auto* pDevice = gD3D12Context->GetDevice();
		pDevice->CreateShaderResourceView(resource, desc, handle.CpuHandle);
		const HRESULT hr = pDevice->GetDeviceRemovedReason();
		return hr;
	}

	__inline D3D12DescriptorHandle CreateUnorderedAccessView(
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
		ID3D12Resource* resource,
		ID3D12Resource* counterBuffer
	)
	{
		auto* pUavHeap = gD3D12Context->GetUAVHeap();
		auto* pDevice = gD3D12Context->GetDevice();

		const auto handle = pUavHeap->Allocate();
		pDevice->CreateUnorderedAccessView(resource, counterBuffer, &desc, handle.CpuHandle);
		return handle;
	}

	__inline HRESULT RefreshUnorderedAccessView(
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource,
		ID3D12Resource* counterBuffer
	)
	{
		auto* pDevice = gD3D12Context->GetDevice();
		pDevice->CreateUnorderedAccessView(resource, counterBuffer, &desc, handle.CpuHandle);
		const HRESULT hr = pDevice->GetDeviceRemovedReason();
		return hr;
	}
}
