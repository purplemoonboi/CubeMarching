#include "D3D12RenderTarget.h"

#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Utilities/D3D12BufferFactory.h"

namespace Foundation::Graphics::D3D12
{
	D3D12RenderTarget::D3D12RenderTarget(
		const void* data, 
		UINT32 width, UINT32 height, 
		TextureFormat format
	)
		:
		Width(width),
		Height(height),
		Depth(1),
		MipLevels(1),
		Format(static_cast<DXGI_FORMAT>(format)),
		RawData(static_cast<BYTE*>(const_cast<void*>(data))),
		Dimension(D3D12_SRV_DIMENSION_TEXTURE2D)
	{

		pResource = D3D12BufferFactory::CreateRenderTexture(Width, Height, Format);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		pSRV = CreateShaderResourceView(srvDesc, pResource.Get());

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = Format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;

		pRTV = CreateRenderTargetView(&rtvDesc, pResource.Get());

		Viewport.TopLeftX = 0;
		Viewport.TopLeftY = 0;
		Viewport.Width = static_cast<float>(Width);
		Viewport.Height = static_cast<float>(Height);
		Viewport.MinDepth = 0.0f;
		Viewport.MaxDepth = 1.0f;

		Rect = { 0, 0, (INT)Width, (INT)Height };
	}

	void D3D12RenderTarget::LoadFromFile(const std::wstring& fileName, const std::string& name)
	{}

	void D3D12RenderTarget::Bind(GraphicsContext* context)
	{}

	void D3D12RenderTarget::UnBind(GraphicsContext* context)
	{}

	void D3D12RenderTarget::OnResize(INT32 width, INT32 height)
	{
		if (Width != width || Height != height)
		{
			Width = width;
			Height = height;

			DirtyFlag = 1;
		}
	}

	UINT64 D3D12RenderTarget::GetWidth() const
	{
		return Width;
	}

	UINT32 D3D12RenderTarget::GetHeight() const
	{
		return Height;
	}

	UINT16 D3D12RenderTarget::GetDepth() const
	{
		return 0;
	}

	TextureDimension D3D12RenderTarget::GetTextureDimension() const
	{
		return static_cast<TextureDimension>(Dimension);
	}

	TextureFormat D3D12RenderTarget::GetTextureFormat() const
	{
		return static_cast<TextureFormat>(Format);
	}

	UINT64 D3D12RenderTarget::GetTexture() const
	{
		return pSRV.GpuHandle.ptr;
	}

	void D3D12RenderTarget::Destroy()
	{
		pResource.Reset();
		
	}

	void D3D12RenderTarget::SetWidth(UINT32 width)
	{
		Width = width;
		DirtyFlag = 1;
	}

	void D3D12RenderTarget::SetHeight(UINT32 height)
	{
		Height = height;
		DirtyFlag = 1;
	}

	void D3D12RenderTarget::SetDepth(UINT16 depth)
	{
		Depth = depth;
		DirtyFlag = 1;
	}

	void D3D12RenderTarget::Regenerate()
	{
		pResource.Reset();

		pResource = D3D12BufferFactory::CreateRenderTexture(Width, Height, Format);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		RefreshShaderResourceViews(&srvDesc, pSRV, pResource.Get());

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = Format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;

		RefreshRenderTargetView(&rtvDesc, pRTV, pResource.Get());

		Viewport.TopLeftX = 0;
		Viewport.TopLeftY = 0;
		Viewport.Width = static_cast<float>(Width);
		Viewport.Height = static_cast<float>(Height);
		Viewport.MinDepth = 0.0f;
		Viewport.MaxDepth = 1.0f;

		Rect = { 0, 0, Width, Height };

		DirtyFlag = 0;
	}
}
