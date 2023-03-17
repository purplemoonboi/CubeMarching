#include "D3D12ResourceManager.h"
#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"

namespace Engine
{
	D3D12ResourceManager::D3D12ResourceManager(ID3D12Device* device)
		:
		Device(device)
	{}

	bool D3D12Context::CreateCbvSrvUavHeap
	(
		UINT opaqueRenderItemCount,
		UINT transparentRenderItemCount,
		UINT voxelWorldResources,
		UINT frameResourceCount
	)
	{
		const UINT objCount = opaqueRenderItemCount;
		// Need a CBV descriptor for each object for each frame resource,
		// +1 for the perPass CBV for each frame resource.
		const UINT numDescriptors = (objCount + 1) * frameResourceCount;

		// Save an offset to the start of the pass CBVs.  These are the last N descriptors.
		PassConstantBufferViewOffset = objCount * frameResourceCount;

		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = numDescriptors;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;

		THROW_ON_FAILURE
		(
			Device->CreateDescriptorHeap
			(
				&cbvHeapDesc,
				IID_PPV_ARGS(&CbvHeap)
			)
		);

		return true;
	}

	bool D3D12ResourceManager::CreateSRVDescriptorHeap
	(
		UINT count, 
		const std::vector<D3D12Texture>& resources
	)
	{
		CORE_ASSERT(!Device, "Device is NULL!");

		/**
		 *			Heap descriptors
		 */
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = count;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		THROW_ON_FAILURE(Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&CbvSrvUavDescriptorHeap)));

		CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(CbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		/**
		 *			Shader resource views
		 */
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;

		for(auto const& resource : resources)
		{
			srvDesc.Format = resource.Format;
			srvDesc.Texture2D.MipLevels = resource.MipLevels;
			Device->CreateShaderResourceView(resource.GpuResource.Get(), &srvDesc, hDescriptor);
			hDescriptor.Offset(1, CbvSrvUavDescriptorSize);
		}

		return true;
	}
}
