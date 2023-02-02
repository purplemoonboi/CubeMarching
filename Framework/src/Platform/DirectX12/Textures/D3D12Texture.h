#pragma once
#include "Platform/DirectX12/DirectX12.h"
#include "Framework/Renderer/Textures/Texture.h"

#include <string>

constexpr SIZE_T MAX_FILE_NAME_MEM_ALLOC = 128;
constexpr SIZE_T MAX_NAME_MEM_ALLOC = 64;

namespace Engine
{
	using Microsoft::WRL::ComPtr;



	class D3D12Texture : public Texture
	{
	public:

		D3D12Texture(const std::wstring& filePath);
		~D3D12Texture() override;

		void Create(UINT32 width, UINT32 height,
			GraphicsContext* context = nullptr,
			TextureDimension dimension = TextureDimension::Two,
			TextureFormat format = TextureFormat::R32G32B32A32
		) override;


		INT32 Width;
		INT32 Height;
		UINT MipLevels;
		std::string Name;
		std::wstring FileName;

		ComPtr<ID3D12Resource> Texture;

		DXGI_FORMAT Format;
		D3D12_SRV_DIMENSION Dimension;

		/**
		 * @brief In essence, out pointers to the resource view on the CPU and GPU.
		 */
		CD3DX12_CPU_DESCRIPTOR_HANDLE ResourceView_CPU_Handle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE ResourceView_GPU_Handle;
	};
}


