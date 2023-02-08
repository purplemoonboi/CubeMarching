#include "D3D12Texture.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include "Platform/DirectX12/Buffers/D3D12BufferUtils.h"

namespace Engine
{


	D3D12Texture::D3D12Texture
	(
		const std::wstring& filePath
	)
		:
		GpuResource(nullptr)
	{
	}

	D3D12Texture::D3D12Texture(UINT64 width, UINT32 height, UINT16 depthOrArrays, TextureDimension dimension, TextureFormat textureFormat)
		:
		Width(width),
		Height(height),
		Depth(depthOrArrays),
		Format(static_cast<DXGI_FORMAT>(textureFormat))
	{
	}

	D3D12Texture::~D3D12Texture()
	{
		GpuResource->Release();
		GpuResource = nullptr;
	}

	void D3D12Texture::InitialiseResource
	(
		const void* initData,
		TextureDimension dimension,
		GraphicsContext* context,
		MemoryManager* memManager
	)
	{
		auto const d3dContext = dynamic_cast<D3D12Context*>(context);
		if (d3dContext != nullptr)
		{
			if (GpuResource != nullptr)
			{
				GpuResource->Release();
				GpuResource = nullptr;
				UploadBuffer->Release();
				UploadBuffer = nullptr;
			}
;
			RawData = static_cast<BYTE*>(const_cast<void*>(initData));

			if(dimension == TextureDimension::Two)
			{
				Dimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				DimensionUav = D3D12_UAV_DIMENSION_TEXTURE2D;

				GpuResource = D3D12BufferUtils::CreateTexture2D(d3dContext->Device.Get(),
					d3dContext->GraphicsCmdList.Get(),
					Width,
					Height,
					initData,
					Format,
					UploadBuffer
				);
			}
			if(dimension == TextureDimension::Three)
			{
				Dimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				DimensionUav = D3D12_UAV_DIMENSION_TEXTURE3D;
				
				GpuResource = D3D12BufferUtils::CreateTexture3D(d3dContext->Device.Get(),
					d3dContext->GraphicsCmdList.Get(),
					Width,
					Height,
					Depth,
					initData,
					Format,
					UploadBuffer
				);
			}

			CreateResourceViews(d3dContext->Device.Get(), dynamic_cast<D3D12MemoryManager*>(memManager));
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
		return (TextureDimension)Dimension;
	}

	TextureFormat D3D12Texture::GetTextureFormat()
	{
		//TODO: Implement a look up table to change between DX12 and Engine formats
		return TextureFormat::R_FLOAT_32;
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

		auto handle = memManager->GetResourceHandle(1);

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

		device->CreateUnorderedAccessView(GpuResource.Get(), nullptr, &uavDesc,
			handle.CpuCurrentHandle.Offset(1, memManager->GetResourceHeapSize()));

			GpuHandleUav = handle.GpuCurrentHandle.Offset(1, memManager->GetResourceHeapSize());

		const HRESULT deviceRemovedReasonUav = device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReasonUav);
	}
}

