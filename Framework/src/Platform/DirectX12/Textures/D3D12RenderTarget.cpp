#include "D3D12RenderTarget.h"

#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"

namespace Engine
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

		GpuResource = D3D12BufferUtils::CreateRenderTexture(Width, Height, Format);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		ResourceSrv = D3D12Utils::CreateShaderResourceView(srvDesc, GpuResource.Get(), ResourceCpuSrv);

		ResourceCpuRtv = D3D12Utils::CreateRenderTargetView(GpuResource.Get(), nullptr);

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
	{
		
	}

	void D3D12RenderTarget::UnBind(GraphicsContext* context)
	{
	}

	void D3D12RenderTarget::OnResize(INT32 width, INT32 height)
	{
		if (Width != width || Height != height)
		{
			Width = width;
			Height = height;

			DirtyFlag = 1;
		}
	}

	UINT64 D3D12RenderTarget::GetWidth()
	{
		return Width;
	}

	UINT32 D3D12RenderTarget::GetHeight()
	{
		return Height;
	}

	UINT16 D3D12RenderTarget::GetDepth()
	{
		return 0;
	}

	TextureDimension D3D12RenderTarget::GetTextureDimension()
	{
		return static_cast<TextureDimension>(Dimension);
	}

	TextureFormat D3D12RenderTarget::GetTextureFormat()
	{
		return static_cast<TextureFormat>(Format);
	}

	UINT64 D3D12RenderTarget::GetTexture()
	{
		return ResourceSrv.ptr;
	}

	void D3D12RenderTarget::Copy(void* src)
	{
	}
	void D3D12RenderTarget::Regenerate()
	{
		GpuResource.Reset();

		GpuResource = D3D12BufferUtils::CreateRenderTexture(Width, Height, Format);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		D3D12Utils::RefreshShaderResourceViews(srvDesc, GpuResource.Get(), ResourceCpuSrv);
		D3D12Utils::RefreshRenderTargetView(GpuResource.Get(), nullptr, ResourceCpuRtv);

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
