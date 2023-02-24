#include "D3D12Texture.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"

namespace Engine
{

	D3D12Texture::~D3D12Texture()
	{
		if (GpuResource != nullptr)
		{
			GpuResource->Release();
			GpuResource = nullptr;
			UploadBuffer->Release();
			UploadBuffer = nullptr;
		}
	}

	D3D12Texture::D3D12Texture
	(
		const void* data, 
		UINT32 width, 
		UINT32 height, 
		UINT16 depthOrArrays, 
		TextureDimension dimension,
		TextureFormat textureFormat
	)
		:
		Width(width),
		Height(height),
		Depth(depthOrArrays),
		Format(static_cast<DXGI_FORMAT>(textureFormat)),
		Dimension(static_cast<D3D12_SRV_DIMENSION>(dimension)),
		DimensionUav(static_cast<D3D12_UAV_DIMENSION>(dimension)),
		GpuResource(nullptr),
		UploadBuffer(nullptr)
	{
		
		if (GpuResource != nullptr)
		{
			GpuResource->Release();
			GpuResource = nullptr;
			UploadBuffer->Release();
			UploadBuffer = nullptr;
		}
		;

		if (dimension == TextureDimension::Tex2D)
		{
			Dimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			DimensionUav = D3D12_UAV_DIMENSION_TEXTURE2D;

			GpuResource = D3D12BufferUtils::CreateTexture2D(
				Width,
				Height,
				data,
				Format,
				UploadBuffer
			);
		}
		if (dimension == TextureDimension::Tex3D)
		{
			Dimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			DimensionUav = D3D12_UAV_DIMENSION_TEXTURE3D;

			GpuResource = D3D12BufferUtils::CreateTexture3D(
				Width,
				Height,
				Depth,
				data,
				Format,
				UploadBuffer
			);
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = Format;
		desc.ViewDimension = Dimension;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = Format;
		uavDesc.ViewDimension = DimensionUav;


		if (Dimension == D3D12_SRV_DIMENSION_TEXTURE2D)
		{
			desc.Texture2D.MipLevels = MipLevels;
			desc.Texture2D.MostDetailedMip = 0;
			uavDesc.Texture2D.MipSlice = 0;
			uavDesc.Texture2D.PlaneSlice = 0;
		}
		else if (Dimension == D3D12_SRV_DIMENSION_TEXTURE3D)
		{
			if (MipLevels != -1)
				MipLevels = -1;

			desc.Texture3D.MipLevels = MipLevels;
			desc.Texture3D.MostDetailedMip = 0;
			uavDesc.Texture3D.MipSlice = 0;
			uavDesc.Texture3D.FirstWSlice = 0;
		}

		GpuHandleSrv = D3D12Utils::CreateShaderResourceView(desc, GpuResource.Get());
		GpuHandleUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, GpuResource.Get());
		
	}

	void D3D12Texture::LoadFromFile(const std::wstring& fileName)
	{
	}

	UINT64 D3D12Texture::GetWidth()
	{
		return Width;
	}

	UINT32 D3D12Texture::GetHeight()
	{
		return Height;
	}

	UINT16 D3D12Texture::GetDepth()
	{
		return Depth;
	}

	TextureDimension D3D12Texture::GetTextureDimension()
	{
		return static_cast<TextureDimension>(Dimension);
	}

	TextureFormat D3D12Texture::GetTextureFormat()
	{
		return static_cast<TextureFormat>(Format);
	}

}

