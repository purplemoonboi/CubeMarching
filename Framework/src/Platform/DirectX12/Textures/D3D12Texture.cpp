#include "D3D12Texture.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/D3D12/D3D12.h"
#include "Platform/DirectX12/Heap/D3D12HeapManager.h"
#include "Platform/DirectX12/Utilities/D3D12BufferFactory.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Loader/D3D12TextureLoader.h"

namespace Foundation::Graphics::D3D12
{

	D3D12Texture::~D3D12Texture()
	{
		if (GpuResource != nullptr)
		{
			gD3D12Context->DeferredRelease(GpuResource.Get());
		}
	}

	D3D12Texture::D3D12Texture
	(
		const void* initData,
		UINT32 width,
		UINT32 height,
		ResourceFormat format
	)
		:
		Texture(width, height, 1, format)
		, GpuResource(nullptr)
		, UploadBuffer(nullptr)
	{

		if (MipLevels != 1)
			MipLevels = 1;


		GpuResource = D3D12BufferFactory::CreateTexture2D(
			Width,
			Height,
			MipLevels,
			initData,
			FoundationResourceFormatToDX12(Format),
			UploadBuffer
		);

		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = Format;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Texture2D.MipLevels = MipLevels;
		desc.Texture2D.MostDetailedMip = 0;

		pSRV = CreateShaderResourceView(desc, GpuResource.Get());

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = Format;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		uavDesc.Texture2D.PlaneSlice = 0;

		pUAV = CreateUnorderedAccessView(uavDesc, GpuResource.Get());
	}

	void D3D12Texture::Destroy()
	{

	}

	void D3D12Texture::SetWidth(UINT32 width)
	{
		Width = width;
		DirtyFlag = 1;
	}

	void D3D12Texture::SetHeight(UINT32 height)
	{
		Height = height;
		DirtyFlag = 1;
	}

	void D3D12Texture::SetDepth(UINT16 depth)
	{
		Depth = depth;
		DirtyFlag = 1;
	}

	UINT64 D3D12Texture::GetTexture() const
	{
		return pSRV.CpuHandle.ptr;
	}

}

