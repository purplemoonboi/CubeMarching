#include "D3D12Utilities.h"
#include "Framework/Core/Core.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/_D3D12ConstantExpressions/D3D12ConstantExpressions.h"

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
		const auto start = Context->RtvHeap->GetCPUDescriptorHandleForHeapStart();
		if(Context->ValidRtvDescriptors < RTV_HEAP_DESC_COUNT)
		{
			const auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(start, ++Context->ValidRtvDescriptors, Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			
			Context->Device->CreateRenderTargetView(resource, nullptr, handle);
			const HRESULT deviceRemovedReason = Context->Device->GetDeviceRemovedReason();
			THROW_ON_FAILURE(deviceRemovedReason);
			return handle;

		}

		return {};
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

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12Utils::CreateDepthStencilView(ID3D12Resource* resource)
	{
		const auto start = Context->DsvHeap->GetCPUDescriptorHandleForHeapStart();

		if(Context->ValidDsvDescriptors < DSV_HEAP_DESC_COUNT)
		{
			const auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(start, ++Context->ValidDsvDescriptors, Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));

			// Create DSV to resource so we can render to the shadow map.
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			dsvDesc.Texture2D.MipSlice = 0;
			Context->Device->CreateDepthStencilView(resource, &dsvDesc, handle);
			return handle;
		}

		return {};
	}

	HRESULT D3D12Utils::RefreshDepthStencilView
	(
		ID3D12Resource* resource, 
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle
	)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;
		Context->Device->CreateDepthStencilView(resource, &dsvDesc, cpuHandle);
		return S_OK;
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
