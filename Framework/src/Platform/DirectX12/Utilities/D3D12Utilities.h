#pragma once
#include "Platform/DirectX12/DirectX12.h"

namespace Foundation::Graphics::D3D12
{
	// Forward declarations.
	class D3D12DescriptorHandle;


	static D3D12DescriptorHandle CreateRenderTargetView(
		const D3D12_RENDER_TARGET_VIEW_DESC* desc,
		ID3D12Resource* resource
	);

	static HRESULT RefreshRenderTargetView(
		const D3D12_RENDER_TARGET_VIEW_DESC* desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource
	);

	static D3D12DescriptorHandle CreateShaderResourceView(
		const D3D12_SHADER_RESOURCE_VIEW_DESC& desc,
		ID3D12Resource* resource
	);

	static HRESULT RefreshShaderResourceViews(
		const D3D12_SHADER_RESOURCE_VIEW_DESC* desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource
	);

	static D3D12DescriptorHandle CreateUnorderedAccessView(
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, 
		ID3D12Resource* resource, 
		ID3D12Resource* counterBuffer = nullptr
	);

	static HRESULT RefreshUnorderedAccessView(
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
		D3D12DescriptorHandle& handle,
		ID3D12Resource* resource, 
		ID3D12Resource* counterBuffer = nullptr
	);

	
}