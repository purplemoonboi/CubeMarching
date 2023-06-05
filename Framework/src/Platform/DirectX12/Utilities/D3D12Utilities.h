#pragma once
#include "Platform/DirectX12/DirectX12.h"

namespace Foundation
{
	class D3D12Context;

	class D3D12HeapManager;
	

	class D3D12ResourceFactory
	{
	public:
		static void Init(D3D12HeapManager* memManager, D3D12Context* context);

		static D3D12_CPU_DESCRIPTOR_HANDLE CreateRenderTargetView(ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc);
		static HRESULT RefreshRenderTargetView(ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc,D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);

		static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ID3D12Resource* resource);
		static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceView(
			const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, 
			ID3D12Resource* resource,
			CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle);

		static HRESULT RefreshShaderResourceViews(
			const D3D12_SHADER_RESOURCE_VIEW_DESC& desc,
			ID3D12Resource* resource,
			CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle
		);
		

		static D3D12_GPU_DESCRIPTOR_HANDLE CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, ID3D12Resource* resource, ID3D12Resource* counterBuffer = nullptr);
		static D3D12_GPU_DESCRIPTOR_HANDLE CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
			ID3D12Resource* resource, 
			CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
			ID3D12Resource* counterBuffer = nullptr);

	private:
		static D3D12Context* Context;
		static D3D12HeapManager* MemoryManager;
	};
}