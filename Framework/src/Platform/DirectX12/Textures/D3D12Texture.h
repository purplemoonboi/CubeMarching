#pragma once
#include <Framework/Renderer/Textures/Texture.h>

#include "Platform/DirectX12/Heap/D3D12HeapManager.h"


namespace Foundation::Graphics::D3D12
{

	class D3D12Texture : public Texture
	{
	public:

		D3D12Texture
		(
			const void* initData,
			UINT32 width,
			UINT32 height,
			UINT16 depth,
			ResourceFormat format
		);

		D3D12Texture
		(
			const void* initData,
			UINT32 width,
			UINT32 height,
			ResourceFormat format = ResourceFormat::RGBA_UINT_UNORM
		);

		D3D12Texture
		(
			const std::wstring& fileName,
			const std::string& name
		);

		~D3D12Texture() override;


		void LoadFromFile(const std::wstring& fileName, const std::string& name) override;

		void Destroy() override;

		void SetWidth(UINT32 width) override;
		void SetHeight(UINT32 height) override;
		void SetDepth(UINT16 depth) override;

		[[nodiscard]] UINT64			GetWidth()				const override;
		[[nodiscard]] UINT32			GetHeight()				const override;
		[[nodiscard]] UINT16			GetDepth()				const override;
		[[nodiscard]] UINT64			GetTexture()			const override;
		[[nodiscard]] TextureDimension	GetTextureDimension()	const override;
		[[nodiscard]] ResourceFormat		GetTextureFormat()		const override;

	private:


		ComPtr<ID3D12Resource> GpuResource;
		ComPtr<ID3D12Resource> UploadBuffer;
		BYTE* RawData = nullptr;

		D3D12DescriptorHandle pSRV;
		D3D12DescriptorHandle pUAV;

		D3D12_SRV_DIMENSION Dimension;
		INT32 SrvIndex = -1;

		D3D12_UAV_DIMENSION DimensionUav;
		INT32 UavIndex = -1;

		UINT8 DirtyFlag{ 0 };

	};
}


