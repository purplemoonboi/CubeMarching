#pragma once
#include <Framework/Renderer/Textures/RenderTarget.h>
#include "Platform/DirectX12/Heap/D3D12HeapManager.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"


namespace Foundation::Graphics::D3D12
{

	class D3D12RenderTarget : public RenderTarget
	{
	public:

		D3D12RenderTarget(const void* data, UINT32 width, UINT32 height, TextureFormat format = TextureFormat::RGBA_UINT_UNORM);

		void LoadFromFile
		(
			const std::wstring& fileName,
			const std::string& name
		) override;

		void Bind(GraphicsContext* context)			override;
		void UnBind(GraphicsContext* context)		override;
		void OnResize(INT32 width, INT32 height)	override;
		void Destroy()								override;

		void SetWidth(UINT32 width) override;
		void SetHeight(UINT32 height) override;
		void SetDepth(UINT16 depth) override;

	public:/*...Getters...*/
		[[nodiscard]] UINT64 GetWidth()							const override;
		[[nodiscard]] UINT32 GetHeight()						const override;
		[[nodiscard]] UINT16 GetDepth()							const override;
		[[nodiscard]] TextureDimension GetTextureDimension()	const override;
		[[nodiscard]] TextureFormat GetTextureFormat()			const override;
		[[nodiscard]] UINT64 GetTexture()						const override;

		ComPtr<ID3D12Resource> pResource;

		void Regenerate();
		INT8 DirtyFlag = 0;

		D3D12_RECT Rect;
		D3D12_VIEWPORT Viewport;

		D3D12DescriptorHandle pSRV;
		D3D12DescriptorHandle pUAV;
		D3D12DescriptorHandle pRTV;

	private:

		INT32 Width;
		INT32 Height;
		UINT16 Depth = 0;
		UINT MipLevels = 1;
		std::string Name;
		std::wstring FileName;

		DXGI_FORMAT Format;

		BYTE* RawData = nullptr;

		INT32 SrvIndex = -1;
		D3D12_SRV_DIMENSION Dimension;



	};

}
