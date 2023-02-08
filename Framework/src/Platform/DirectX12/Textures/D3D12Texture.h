#pragma once
#include "Platform/DirectX12/DirectX12.h"
#include "Framework/Renderer/Textures/Texture.h"

#include <string>

constexpr SIZE_T MAX_FILE_NAME_MEM_ALLOC = 128;
constexpr SIZE_T MAX_NAME_MEM_ALLOC = 64;

namespace Engine
{
	class D3D12MemoryManager;
	using Microsoft::WRL::ComPtr;



	class D3D12Texture : public Texture
	{
	public:

		D3D12Texture(const std::wstring& filePath);
		D3D12Texture(UINT64 width, UINT32 height, UINT16 depthOrArrays, TextureDimension dimension, TextureFormat textureFormat);
		~D3D12Texture() override;

		void InitialiseResource(
			const void* initData,
			TextureDimension dimension = TextureDimension::Two,
			GraphicsContext* context = nullptr,
			MemoryManager* memManager = nullptr
		) override;

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
	private:

		void CreateResourceViews(ID3D12Device* device, D3D12MemoryManager* memManager);


	};
}


