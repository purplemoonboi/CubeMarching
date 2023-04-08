#include "D3D12Utilities.h"
#include "Framework/Core/Core.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include "Platform/DirectX12/Api/D3D12Context.h"


namespace Engine
{
	
	D3D12MemoryManager* D3D12Utils::MemoryManager = nullptr;
	D3D12Context* D3D12Utils::Context = nullptr;

	void D3D12Utils::Init(D3D12MemoryManager* memManager, D3D12Context* context)
	{
		Context = context;
		MemoryManager = memManager;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12Utils::CreateRenderTargetView(
		ID3D12Resource* resource,
		const D3D12_RENDER_TARGET_VIEW_DESC* desc
	)
	{
		auto start = Context->RtvHeap->GetCPUDescriptorHandleForHeapStart();
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(start, 2, Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		D3D12_CLEAR_VALUE clearVal = {};
		clearVal.Color[0] = DirectX::Colors::SandyBrown[0];
		clearVal.Color[1] = DirectX::Colors::SandyBrown[1];
		clearVal.Color[2] = DirectX::Colors::SandyBrown[2];
		clearVal.Color[3] = DirectX::Colors::SandyBrown[3];
		clearVal.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		
		Context->Device->CreateRenderTargetView(resource, nullptr, handle);
		const HRESULT deviceRemovedReason = Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);
		return handle;
	}

	HRESULT D3D12Utils::RefreshRenderTargetView(
		ID3D12Resource* resource, 
		const D3D12_RENDER_TARGET_VIEW_DESC* desc,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle
	)
	{
		Context->Device->CreateRenderTargetView(resource, desc, cpuHandle);
		const HRESULT deviceRemovedReason = Context->Device->GetDeviceRemovedReason();
		return deviceRemovedReason;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12Utils::CreateShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ID3D12Resource* resource)
	{
		const auto handles = MemoryManager->GetResourceHandle();
		Context->Device->CreateShaderResourceView(resource, &desc, handles.CpuCurrentHandle);
		const HRESULT deviceRemovedReason = Context->Device->GetDeviceRemovedReason();
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
		Context->Device->CreateShaderResourceView(resource, &desc, handles.CpuCurrentHandle);
		const HRESULT deviceRemovedReason = Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);
		return handles.GpuCurrentHandle;
	}

	HRESULT D3D12Utils::RefreshShaderResourceViews(
		const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, 
		ID3D12Resource* resource,
		CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle
	)
	{
		Context->Device->CreateShaderResourceView(resource, &desc, cpuHandle);
		const HRESULT deviceRemovedReason = Context->Device->GetDeviceRemovedReason();
		return deviceRemovedReason;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12Utils::CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, ID3D12Resource* resource, ID3D12Resource* counterBuffer)
	{
		const auto handles = MemoryManager->GetResourceHandle();
		Context->Device->CreateUnorderedAccessView(resource, counterBuffer, &desc, handles.CpuCurrentHandle);
		const HRESULT deviceRemovedReason = Context->Device->GetDeviceRemovedReason();
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
		Context->Device->CreateUnorderedAccessView(resource, counterBuffer, &desc, handles.CpuCurrentHandle);
		const HRESULT deviceRemovedReason = Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);
		return handles.GpuCurrentHandle;
	}
}
