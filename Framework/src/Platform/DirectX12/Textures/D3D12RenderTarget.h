#pragma once
#include <Framework/Renderer/Textures/RenderTarget.h>

#include "Platform/DirectX12/DirectX12.h"


namespace Foundation
{
	using Microsoft::WRL::ComPtr;

	class D3D12RenderTarget : public RenderTarget
	{
	public:

		D3D12RenderTarget(const void* data, UINT32 width, UINT32 height, TextureFormat format = TextureFormat::RGBA_UINT_UNORM);

		void LoadFromFile
		(
			const std::wstring& fileName,
			const std::string& name
		) override;

		void Bind(GraphicsContext* context) override;
		void UnBind(GraphicsContext* context) override;
		void OnResize(INT32 width, INT32 height) override;

		UINT64 GetWidth() override;
		UINT32 GetHeight() override;
		UINT16 GetDepth() override;
		TextureDimension GetTextureDimension() override;
		TextureFormat GetTextureFormat() override;
		UINT64 GetTexture() override;
		void Destroy() override;
		void Copy(void* src) override;

		INT32 Width;
		INT32 Height;
		UINT16 Depth = 0;
		UINT MipLevels = 1;
		std::string Name;
		std::wstring FileName;

		DXGI_FORMAT Format;

		ComPtr<ID3D12Resource> pResource;
		BYTE* RawData = nullptr;

		D3D12_SRV_DIMENSION Dimension;

		CD3DX12_GPU_DESCRIPTOR_HANDLE ResourceSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE ResourceCpuSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE pRTV;

		INT32 SrvIndex = -1;

		D3D12_RECT Rect;
		D3D12_VIEWPORT Viewport;

		void Regenerate();
		INT8 DirtyFlag = 0;

	};

}
