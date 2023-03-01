#pragma once
#include "Platform/DirectX12/DirectX12.h"
#include "Framework/Renderer/Textures/Texture.h"

#include <string>


namespace Engine
{
	class D3D12MemoryManager;
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
			TextureFormat format
		);
		~D3D12Texture() override;


		void LoadFromFile(const std::wstring& fileName) override;

		UINT64 GetWidth() override;
		UINT32 GetHeight() override;
		UINT16 GetDepth() override;
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


