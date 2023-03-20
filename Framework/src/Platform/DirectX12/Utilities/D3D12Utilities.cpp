#include "D3D12Utilities.h"
#include "Framework/Core/Core.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"


namespace Engine
{
	ID3D12Device* D3D12Utils::Device = nullptr;
	D3D12MemoryManager* D3D12Utils::MemoryManager = nullptr;

	void D3D12Utils::Init(ID3D12Device* device, D3D12MemoryManager* memManager)
	{
		Device = device;
		MemoryManager = memManager;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12Utils::CreateShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ID3D12Resource* resource)
	{
		const auto handles = MemoryManager->GetResourceHandle();
		Device->CreateShaderResourceView(resource, &desc, handles.CpuCurrentHandle);
		const HRESULT deviceRemovedReason = Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);
		return handles.GpuCurrentHandle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12Utils::CreateShaderResourceView(
		const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, 
		ID3D12Resource* resource,
		CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle
	)
	{
		const auto handles = MemoryManager->GetResourceHandle();
		cpuHandle = handles.CpuCurrentHandle;
		Device->CreateShaderResourceView(resource, &desc, handles.CpuCurrentHandle);
		const HRESULT deviceRemovedReason = Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);
		return handles.GpuCurrentHandle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12Utils::CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, ID3D12Resource* resource, ID3D12Resource* counterBuffer)
	{
		const auto handles = MemoryManager->GetResourceHandle();
		Device->CreateUnorderedAccessView(resource, counterBuffer, &desc, handles.CpuCurrentHandle);
		const HRESULT deviceRemovedReason = Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);
		return handles.GpuCurrentHandle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12Utils::CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc,
		ID3D12Resource* resource, 
		CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, 
		ID3D12Resource* counterBuffer)
	{
		const auto handles = MemoryManager->GetResourceHandle();
		cpuHandle = handles.CpuCurrentHandle;
		Device->CreateUnorderedAccessView(resource, counterBuffer, &desc, handles.CpuCurrentHandle);
		const HRESULT deviceRemovedReason = Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);
		return handles.GpuCurrentHandle;
	}
}
