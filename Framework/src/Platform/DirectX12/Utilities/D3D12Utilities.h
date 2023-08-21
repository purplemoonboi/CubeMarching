#pragma once
#include "Platform/DirectX12/DirectX12.h"

namespace Foundation::Graphics::D3D12
{
	// Forward declarations.
	class D3D12DescriptorHandle;


	D3D12DescriptorHandle CreateRenderTargetView(
		const D3D12_RENDER_TARGET_VIEW_DESC* desc,
		ID3D12Resource* resource
	);

	HRESULT RefreshRenderTargetView(
		const D3D12_RENDER_TARGET_VIEW_DESC* desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource
	);

	D3D12DescriptorHandle CreateDepthStencilView(
		const D3D12_DEPTH_STENCIL_VIEW_DESC* desc,
		ID3D12Resource* resource
	);

	HRESULT RefreshDepthStencilView(
		const D3D12_DEPTH_STENCIL_VIEW_DESC* desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource
	);

	D3D12DescriptorHandle CreateShaderResourceView(
		const D3D12_SHADER_RESOURCE_VIEW_DESC& desc,
		ID3D12Resource* resource
	);

	HRESULT RefreshShaderResourceViews(
		const D3D12_SHADER_RESOURCE_VIEW_DESC* desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource
	);

	D3D12DescriptorHandle CreateUnorderedAccessView(
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, 
		ID3D12Resource* resource, 
		ID3D12Resource* counterBuffer = nullptr
	);

	HRESULT RefreshUnorderedAccessView(
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource, 
		ID3D12Resource* counterBuffer = nullptr
	);

	
}