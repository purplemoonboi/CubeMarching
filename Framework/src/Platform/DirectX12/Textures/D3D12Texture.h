#pragma once
#include "Platform/DirectX12/DirectX12.h"
#include "Framework/Renderer/Textures/Texture.h"

#include <string>


namespace Foundation
{
	class D3D12HeapManager;
	using Microsoft::WRL::ComPtr;



	class D3D12Texture : public Texture
	{
	public:

		D3D12Texture
		(
			const void* initData,
			UINT32 width,
			UINT32 height,
			UINT16 depth,
			TextureFormat format
		);

		D3D12Texture
		(
			const void* initData,
			UINT32 width,
			UINT32 height,
			TextureFormat format = TextureFormat::RGBA_UINT_UNORM
		);

		D3D12Texture
		(
			const std::wstring& fileName,
			const std::string& name
		);

		~D3D12Texture() override;


		void LoadFromFile(const std::wstring& fileName, const std::string& name) override;

		void Destroy() override;


		UINT64 GetWidth() override;
		UINT32 GetHeight() override;
		UINT16 GetDepth() override;
		UINT64 GetTexture() override;
		void Copy(void* src) override;
		TextureDimension GetTextureDimension() override;
		TextureFormat GetTextureFormat() override;

		UINT64 Width;
		UINT32 Height;
		UINT16 Depth;
		UINT MipLevels = -1;
		std::string Name;
		std::wstring FileName;

		DXGI_FORMAT Format;

		ComPtr<ID3D12Resource> GpuResource;
		ComPtr<ID3D12Resource> UploadBuffer;
		BYTE* RawData = nullptr;



		D3D12_SRV_DIMENSION Dimension;
		CD3DX12_GPU_DESCRIPTOR_HANDLE GpuHandleSrv;
		INT32 SrvIndex = -1;

		D3D12_UAV_DIMENSION DimensionUav;
		CD3DX12_GPU_DESCRIPTOR_HANDLE GpuHandleUav;
		INT32 UavIndex = -1;



	};
}


