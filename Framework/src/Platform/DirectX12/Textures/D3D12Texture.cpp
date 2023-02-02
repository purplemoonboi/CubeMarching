#include "D3D12Texture.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Engine
{
	D3D12Texture::D3D12Texture
	(
		const std::wstring& filePath
	)
		:
		Texture(nullptr)
	{
	
	}

	D3D12Texture::~D3D12Texture()
	{
		Texture->Release();
		Texture = nullptr;
	}

	void D3D12Texture::Create
	(
		UINT32 width, UINT32 height, 
		GraphicsContext* context,	
		TextureDimension dimension,
		TextureFormat format
	)
	{
		auto const d3dContext = dynamic_cast<D3D12Context*>(context);
		if (d3dContext != nullptr)
		{
			// Describe and create a Texture2D.
			D3D12_RESOURCE_DESC textureDesc = {};
			textureDesc.MipLevels = 1;
			textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			textureDesc.Width = width;
			textureDesc.Height = height;
			textureDesc.DepthOrArraySize = width;
			textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

			THROW_ON_FAILURE
			(
				d3dContext->Device->CreateCommittedResource
				(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
					D3D12_HEAP_FLAG_NONE,
					&textureDesc,
					D3D12_RESOURCE_STATE_COMMON,
					nullptr,
					IID_PPV_ARGS(&Texture)
				)
			);

		}
	}
}

