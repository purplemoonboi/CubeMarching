#include "D3D12HeapManager.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/_D3D12ConstantExpressions/D3D12ConstantExpressions.h"


namespace Foundation
{

	D3D12HeapManager::D3D12HeapManager(ID3D12Device* device)
		:
		pDevice(device)
	{
		CORE_ASSERT(pDevice, "Device error. Call GetDeviceRemovedReason().");
		CbvSrvUavDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		DsvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		RtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


		CreateRtvAndDsvHeaps();
	}

	D3D12HeapManager::~D3D12HeapManager()
	{
	}

	HRESULT D3D12HeapManager::Init(UINT framesInFlight, UINT maxObjectCount)
	{
		HRESULT hr{ S_OK };
		const UINT objCount = maxObjectCount;
		// Need a CBV descriptor for each object for each frame resource,
		// +1 for the perPass CBV for each frame resource.
		const UINT numDescriptors = (objCount + 1) * framesInFlight;

		// Save an offset to the start of the *pass* CBVs.  These are the last N descriptors.
		PassBufferOffsetSize = objCount * framesInFlight;

		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = numDescriptors;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;

		hr = pDevice->CreateDescriptorHeap
		(
			&cbvHeapDesc,
			IID_PPV_ARGS(&CbvHeap)
		);
		THROW_ON_FAILURE(hr);

		/**
		 * create our SRV heap
		 */
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 31U + 1U;// +1 for ImGui
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		hr = pDevice->CreateDescriptorHeap(&srvHeapDesc,
			IID_PPV_ARGS(&SrvUavHeap));
		THROW_ON_FAILURE(hr);

		ResourceHandles.CpuCurrentHandle = SrvUavHeap->GetCPUDescriptorHandleForHeapStart();
		ResourceHandles.GpuCurrentHandle = SrvUavHeap->GetGPUDescriptorHandleForHeapStart();

		ImGuiHandleOffset = 31U + 1U;
		MaxHandleOffset = ImGuiHandleOffset;
		ImGuiHandles.CpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetCPUDescriptorHandleForHeapStart(), ImGuiHandleOffset, CbvSrvUavDescriptorSize);
		ImGuiHandles.GpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetGPUDescriptorHandleForHeapStart(), ImGuiHandleOffset, CbvSrvUavDescriptorSize);

		IsInitialised = true;
		

		return hr;

	}

	HRESULT D3D12HeapManager::CreateRtvAndDsvHeaps()
	{
		HRESULT hr{ S_OK };

		// Resource 
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT + 1;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;

		hr = pDevice->CreateDescriptorHeap
		(
			&rtvHeapDesc,
			IID_PPV_ARGS(RtvHeap.GetAddressOf())
		);
		THROW_ON_FAILURE(hr);

		// Depth stencil
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;

		hr = pDevice->CreateDescriptorHeap
		(
			&dsvHeapDesc,
			IID_PPV_ARGS(DsvHeap.GetAddressOf())
		);
		THROW_ON_FAILURE(hr);

		return hr;
	}

	D3D12HeapManager::Handles D3D12HeapManager::GetResourceHandle(INT32 off)
	{
	
		const auto offset = HandleOffset;
		HandleOffset += off;

		if(offset == ImGuiHandleOffset)
		{
			return
			{
				CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetGPUDescriptorHandleForHeapStart(), 0, CbvSrvUavDescriptorSize),
				CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetCPUDescriptorHandleForHeapStart(),  0, CbvSrvUavDescriptorSize),
				false
			};
		}

		return {
			 CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetGPUDescriptorHandleForHeapStart(), offset, CbvSrvUavDescriptorSize),
			 CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvUavHeap->GetCPUDescriptorHandleForHeapStart(),  offset, CbvSrvUavDescriptorSize)
		};
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12HeapManager::GetConstantBufferViewCpu() const
	{
		return CbvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12HeapManager::GetConstantBufferViewGpu() const
	{
		return CbvHeap->GetGPUDescriptorHandleForHeapStart();
	}



}

