#include "D3D12Texture.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
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
			/*GpuResource->Release();
			GpuResource = nullptr;
			UploadBuffer->Release();
			UploadBuffer = nullptr;*/
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

		GpuResource = D3D12BufferFactory::CreateTexture3D(
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

		pSRV = CreateShaderResourceView(desc, GpuResource.Get());


		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = Format;
		uavDesc.ViewDimension = DimensionUav;
		uavDesc.Texture3D.MipSlice = 0;
		uavDesc.Texture3D.FirstWSlice = 0;
		uavDesc.Texture3D.WSize = Depth;

		pUAV = CreateUnorderedAccessView(uavDesc, GpuResource.Get());
		
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


		GpuResource = D3D12BufferFactory::CreateTexture2D(
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

		pSRV = CreateShaderResourceView(desc, GpuResource.Get());

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = Format;
		uavDesc.ViewDimension = DimensionUav;
		uavDesc.Texture2D.MipSlice = 0;
		uavDesc.Texture2D.PlaneSlice = 0;

		pUAV = CreateUnorderedAccessView(uavDesc, GpuResource.Get());
	}

	D3D12Texture::D3D12Texture(const std::wstring& fileName, const std::string& name)
	{
		FileName = fileName;
		Name = name;

		const HRESULT hr = D3D12TextureLoader::LoadTexture2DFromFile(fileName,
			GpuResource, UploadBuffer);
		THROW_ON_FAILURE(hr);

		auto dimension = GpuResource->GetDesc().Dimension;

		Width	= GpuResource->GetDesc().Width;
		Height	= GpuResource->GetDesc().Height;
		Depth	= GpuResource->GetDesc().DepthOrArraySize;

		Dimension = (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D) ?
			D3D12_SRV_DIMENSION_TEXTURE1D : (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D) ?
			D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_TEXTURE3D;

		Format = GpuResource->GetDesc().Format;

		DimensionUav = D3D12_UAV_DIMENSION_UNKNOWN;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = Format;
		srvDesc.ViewDimension = Dimension;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 3;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		pSRV = CreateShaderResourceView(srvDesc, GpuResource.Get());
	}

	void D3D12Texture::LoadFromFile(const std::wstring& fileName, const std::string& name)
	{
		FileName = fileName;
		Name = name;

		if(GpuResource != nullptr)
		{
			GpuResource->Release();
			UploadBuffer->Release();
		}

		const HRESULT hr = D3D12TextureLoader::LoadTexture2DFromFile(fileName,
			GpuResource, UploadBuffer);
		THROW_ON_FAILURE(hr);

		auto dimension = GpuResource->GetDesc().Dimension;

		Width  = GpuResource->GetDesc().Width;
		Height = GpuResource->GetDesc().Height;
		Depth  = GpuResource->GetDesc().DepthOrArraySize;

		Dimension = (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D) ?
			D3D12_SRV_DIMENSION_TEXTURE1D : (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D) ?
			D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_TEXTURE3D;

		Format = GpuResource->GetDesc().Format;

		DimensionUav = D3D12_UAV_DIMENSION_UNKNOWN;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = Format;
		srvDesc.ViewDimension = Dimension;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = GpuResource->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		pSRV = CreateShaderResourceView(srvDesc, GpuResource.Get());
	}

	void D3D12Texture::Destroy()
	{
		GpuResource.Reset();
		GpuResource = nullptr;
		UploadBuffer.Reset();
		UploadBuffer = nullptr;
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

	UINT64 D3D12Texture::GetWidth() const
	{
		return Width;
	}

	UINT32 D3D12Texture::GetHeight() const
	{
		return Height;
	}

	UINT16 D3D12Texture::GetDepth() const
	{
		return Depth;
	}

	UINT64 D3D12Texture::GetTexture() const
	{
		return pSRV.CpuHandle.ptr;
	}

	TextureDimension D3D12Texture::GetTextureDimension() const
	{
		return static_cast<TextureDimension>(Dimension);
	}

	TextureFormat D3D12Texture::GetTextureFormat() const
	{
		return static_cast<TextureFormat>(Format);
	}

}

