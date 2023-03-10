#include "D3D12Texture.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"

#include "Platform/DirectX12/Copy/D3D12CopyContext.h"

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
		TextureFormat textureFormat
	)
		:
		Width(width),
		Height(height),
		Depth(depthOrArrays),
		Format(static_cast<DXGI_FORMAT>(textureFormat)),
		GpuResource(nullptr),
		UploadBuffer(nullptr)

	{
		Dimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		DimensionUav = D3D12_UAV_DIMENSION_TEXTURE3D;

		if (MipLevels != 1)
			MipLevels = 1;

		GpuResource = D3D12BufferUtils::CreateTexture3D(
			Width,
			Height,
			Depth,
			MipLevels,
			data,
			Format,
			UploadBuffer
		);
		GpuResource.Get()->SetName(L"Texture 2D");

		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = Format;
		desc.ViewDimension = Dimension;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Texture3D.MipLevels = MipLevels;
		desc.Texture3D.MostDetailedMip = 0;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = Format;
		uavDesc.ViewDimension = DimensionUav;
		uavDesc.Texture3D.MipSlice = 0;
		uavDesc.Texture3D.FirstWSlice = 0;
		uavDesc.Texture3D.WSize = Depth;

		GpuHandleSrv = D3D12Utils::CreateShaderResourceView(desc, GpuResource.Get());
		GpuHandleUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, GpuResource.Get());
		
	}

	D3D12Texture::D3D12Texture
	(
		const void* initData, 
		UINT32 width, 
		UINT32 height,
		TextureFormat format
	)
		:
		Width(width),
		Height(height),
		Format(static_cast<DXGI_FORMAT>(format)),
		GpuResource(nullptr),
		UploadBuffer(nullptr)
	{
		Depth = 0;
		Dimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		DimensionUav = D3D12_UAV_DIMENSION_TEXTURE2D;

		if (MipLevels != 1)
			MipLevels = 1;


		GpuResource = D3D12BufferUtils::CreateTexture2D(
			Width,
			Height,
			MipLevels,
			initData,
			Format,
			UploadBuffer
		);



		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = Format;
		desc.ViewDimension = Dimension;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Texture2D.MipLevels = MipLevels;
		desc.Texture2D.MostDetailedMip = 0;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = Format;
		uavDesc.ViewDimension = DimensionUav;
		uavDesc.Texture2D.MipSlice = 0;
		uavDesc.Texture2D.PlaneSlice = 0;

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

	UINT64 D3D12Texture::GetTexture()
	{
		return GpuHandleSrv.ptr;
	}

	void D3D12Texture::Copy(void* src)
	{

		ID3D12Resource* _src = static_cast<ID3D12Resource*>(src);

		/**
		 * Add a copy command to the copy context.
		 * @note - copying does not happen immediately, ensure the copy
		 *		   is in scope
		 */
		D3D12CopyContext::CopyTexture(_src, GpuResource.Get(), 0, 0, 0);

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

