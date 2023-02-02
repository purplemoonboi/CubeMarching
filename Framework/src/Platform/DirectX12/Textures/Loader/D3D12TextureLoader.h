#pragma once
#include <map>
#include <memory>
#include <string>
#include "../../vendor/Microsoft/DDSTextureLoader.h"
#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Engine
{
	class TextureManager
	{
		friend class D3D12TextureLoader;

	public:

		bool InsertTexture(const std::string& name, D3D12Texture& texture)
		{
			
			return false;
		}

		bool RemoveTexture(const std::string& name)
		{
			
			return true;
		}

		D3D12Texture& GetTexture2D(const std::string& name)
		{
			
		}

	private:

		static std::map<std::string, Texture> TextureLib;

	}; std::map<std::string, Texture> TextureManager::TextureLib;

	class D3D12TextureLoader
	{

		friend class TextureManager;

	public:

		static bool LoadTexture2DFromFile
		(
			const std::wstring& fileName,
			const std::string& name,
			D3D12Context* context
		)
		{
		
			return true;
		}

	};
}
