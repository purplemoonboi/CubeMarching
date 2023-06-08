#include "D3D12Utilities.h"
#include "Framework/Core/Core.h"
#include "Framework/Core/Log/Log.h"

#include "Platform/DirectX12/Core/D3D12Core.h"
#include "Platform/DirectX12/Heap/D3D12HeapManager.h"

namespace Foundation::Graphics::D3D12
{

	D3D12DescriptorHandle CreateRenderTargetView(
		const D3D12_RENDER_TARGET_VIEW_DESC* desc,
		ID3D12Resource* resource
	)
	{
		const auto handle = RtvHeap.Allocate();
		pDevice->CreateRenderTargetView(resource, nullptr, handle.CpuHandle);
		const HRESULT deviceRemovedReason = pDevice->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);
		return handle;
	}

	HRESULT RefreshRenderTargetView(
		const D3D12_RENDER_TARGET_VIEW_DESC* desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource
	)
	{
		pDevice->CreateRenderTargetView(resource, desc, handle.CpuHandle);
		const HRESULT hr = pDevice->GetDeviceRemovedReason();
		return hr;
	}

	D3D12DescriptorHandle CreateShaderResourceView(
		const D3D12_SHADER_RESOURCE_VIEW_DESC& desc,
		ID3D12Resource* resource
	)
	{
		const auto handle = SrvHeap.Allocate();
		pDevice->CreateShaderResourceView(resource, &desc, handle.CpuHandle);
		const HRESULT deviceRemovedReason = pDevice->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);
		return handle;
	}

	HRESULT RefreshShaderResourceViews(
		const D3D12_SHADER_RESOURCE_VIEW_DESC* desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource
	)
	{
		pDevice->CreateShaderResourceView(resource, desc, handle.CpuHandle);
		const HRESULT hr = pDevice->GetDeviceRemovedReason();
		return hr;
	}

	D3D12DescriptorHandle CreateUnorderedAccessView(
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
		ID3D12Resource* resource,
		ID3D12Resource* counterBuffer
	)
	{
		const auto handle = UavHeap.Allocate();
		pDevice->CreateUnorderedAccessView(resource, counterBuffer, &desc, handle.CpuHandle);
		const HRESULT hr = pDevice->GetDeviceRemovedReason();
		THROW_ON_FAILURE(hr);
		return handle;
	}

	HRESULT RefreshUnorderedAccessView(
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource,
		ID3D12Resource* counterBuffer
	)
	{
		pDevice->CreateUnorderedAccessView(resource, counterBuffer, &desc, handle.CpuHandle);
		const HRESULT hr = pDevice->GetDeviceRemovedReason();
		THROW_ON_FAILURE(hr);
		return hr;
	}
}
