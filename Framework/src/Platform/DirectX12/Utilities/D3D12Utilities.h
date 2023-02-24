#pragma once
#include "Platform/DirectX12/DirectX12.h"

namespace Engine
{

	class D3D12MemoryManager;
	

	class D3D12Utils
	{
	public:
		static void Init(ID3D12Device* device, D3D12MemoryManager* memManager);

		static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ID3D12Resource* resource);
		static D3D12_GPU_DESCRIPTOR_HANDLE CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, ID3D12Resource* resource, ID3D12Resource* counterBuffer = nullptr);

	private:
		static ID3D12Device* Device;
		static D3D12MemoryManager* MemoryManager;
	};
}