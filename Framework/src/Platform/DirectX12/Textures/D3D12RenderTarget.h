#pragma once
#include <Framework/Renderer/Textures/RenderTarget.h>
#include "Platform/DirectX12/Heap/D3D12HeapManager.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"


namespace Foundation::Graphics::D3D12
{

	class D3D12RenderTarget : public RenderTarget
	{
	public:
		explicit D3D12RenderTarget(ResourceFormat format, UINT32 width = 512, UINT32 height = 512);
		DISABLE_COPY(D3D12RenderTarget);
		D3D12RenderTarget(D3D12RenderTarget&& rhs) noexcept;
		auto operator=(D3D12RenderTarget&& rhs) noexcept -> D3D12RenderTarget&;
		~D3D12RenderTarget() override;

		void LoadFromFile(const std::wstring& fileName, const std::string& name) override;
		void Bind()										override;
		void OnResize(UINT32 width, UINT32 height)		override;
		void OnDestroy()								override;
		void SetWidth(UINT32 width)						override;
		void SetHeight(UINT32 height)					override;

	public:/*...Getters...*/
		[[nodiscard]] UINT32 GetWidth()					const override;
		[[nodiscard]] UINT32 GetHeight()				const override;
		[[nodiscard]] ResourceFormat GetTextureFormat()	const override;
		[[nodiscard]] void* GetTexture()				const override;


	private:
		std::string Name{ "RenderTarget" };
		std::wstring FileName{ L"empty" };

		ComPtr<ID3D12Resource> pResource{ nullptr };

		INT32 Width{ -1 };
		INT32 Height{ -1 };

		D3D12DescriptorHandle pSRV{};
		D3D12DescriptorHandle pRTV{};
		D3D12DescriptorHandle pDSV{};


		D3D12_RECT Rect{};
		D3D12_VIEWPORT Viewport{};
		UINT MipLevels = 1;
		DXGI_FORMAT Format{ DXGI_FORMAT_R8G8B8A8_UNORM };
		D3D12_SRV_DIMENSION Dimension{ D3D12_SRV_DIMENSION_TEXTURE2D };
		INT8 DirtyFlag = 0;
		BYTE* RawData = nullptr;
	};

}
