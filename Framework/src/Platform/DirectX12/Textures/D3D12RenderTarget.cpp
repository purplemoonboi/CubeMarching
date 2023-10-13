#include "D3D12RenderTarget.h"

#include "Platform/DirectX12/D3D12/D3D12.h"
#include "Platform/DirectX12/Utilities/D3D12BufferFactory.h"

namespace Foundation::Graphics::D3D12
{
	D3D12RenderTarget::D3D12RenderTarget(ResourceFormat format, UINT32 width, UINT32 height)
		:
		Width(width),
		Height(height),
		Format(static_cast<DXGI_FORMAT>(format)),
		RawData(static_cast<BYTE*>(const_cast<void*>(data)))
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

	D3D12RenderTarget::D3D12RenderTarget(D3D12RenderTarget&& rhs) noexcept
	{
		Name = std::move(rhs.Name);
		FileName = std::move(rhs.FileName);
		Width = rhs.Width;
		Height = rhs.Height;

		rhs.Width = -1;//Force max value
		rhs.Width = -1;//so render it garbage.

		Rect = std::move(rhs.Rect);
		Viewport = std::move(rhs.Viewport);
	}

	auto D3D12RenderTarget::operator=(D3D12RenderTarget&& rhs) noexcept -> D3D12RenderTarget&
	{
		*this = std::move(rhs);
		return *this;
	}

	D3D12RenderTarget::~D3D12RenderTarget()
	{
		const auto rtvHeap = gD3D12Context->GetRTVHeap();
		rtvHeap->Free(pRTV);

		gD3D12Context->DeferredRelease(pResource.Get());
	}

	void D3D12RenderTarget::Bind()
	{
		const auto frame = gD3D12Context->CurrentRenderFrame();
		CORE_ASSERT(frame->GetFrameGraphicsCommandList(), "Invalid graphics command list!");


		frame->GetFrameGraphicsCommandList()->OMSetRenderTargets(1, &pRTV.CpuHandle, FALSE, &pDSV.CpuHandle);
	}

	void D3D12RenderTarget::OnDestroy()
	{
		auto* srvHeap = gD3D12Context->GetSRVHeap();
		auto* rtvHeap = gD3D12Context->GetRTVHeap();
		auto* dsvHeap = gD3D12Context->GetDSVHeap();

		rtvHeap->Free(pRTV);
		srvHeap->Free(pSRV);
		dsvHeap->Free(pRTV);

		gD3D12Context->DeferredRelease(pResource.Get());
	}

	void D3D12RenderTarget::LoadFromFile(const std::wstring& fileName, const std::string& name)
	{}

	void D3D12RenderTarget::OnResize(UINT32 width, UINT32 height)
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

	UINT32 D3D12RenderTarget::GetWidth() const
	{
		return Width;
	}

	UINT32 D3D12RenderTarget::GetHeight() const
	{
		return Height;
	}

	ResourceFormat D3D12RenderTarget::GetTextureFormat() const
	{
		return static_cast<ResourceFormat>(Format);
	}

	void* D3D12RenderTarget::GetTexture() const
	{
		return reinterpret_cast<void*>(pSRV.GpuHandle.ptr);
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

}
