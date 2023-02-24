#include "D3D12Texture.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include "Platform/DirectX12/Buffers/D3D12BufferUtils.h"

namespace Engine
{

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

		CreateResourceViews(d3dContext->Device.Get(), dynamic_cast<D3D12MemoryManager*>(memManager));
		
	}

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

	void D3D12Texture::CreateResourceViews(ID3D12Device* device, D3D12MemoryManager* memManager)
	{

		/**
		 * create a shader resource view.
		 */
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = Format;
		srvDesc.ViewDimension = Dimension;
		srvDesc.Texture3D.MipLevels = 1;
		srvDesc.Texture3D.MostDetailedMip = 0;

		auto handle = memManager->GetResourceHandle();

		device->CreateShaderResourceView(GpuResource.Get(), &srvDesc, handle.CpuCurrentHandle);
		GpuHandleSrv = handle.GpuCurrentHandle;


		/**
		 * create an unordered access view into the same resource.
		 */
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = Format;
		const HRESULT deviceRemovedReasonSrv = device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReasonSrv);
		uavDesc.ViewDimension = DimensionUav;
		uavDesc.Texture3D.MipSlice = 0;
		uavDesc.Texture3D.WSize = Depth;
		uavDesc.Texture3D.FirstWSlice = 0;

		/*
		* offset again to the next descriptor table
		*/
		handle = memManager->GetResourceHandle();

		device->CreateUnorderedAccessView(GpuResource.Get(), nullptr, &uavDesc, 
			handle.CpuCurrentHandle);

		GpuHandleUav = handle.GpuCurrentHandle;

		const HRESULT deviceRemovedReasonUav = device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReasonUav);
	}
}

