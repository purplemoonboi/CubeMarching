#include "Texture.h"
#include "Framework/Renderer/Api/RendererAPI.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"

namespace Foundation
{
	std::unordered_map<TextureFormat, UINT64> TextureTypeSize::TypeSize = {};

	ScopePointer<Texture> Texture::Create
	(
		const void* initData,
		UINT32 width,
		UINT32 height,
		UINT16 depth,
		TextureFormat format 
	)
	{
		switch(RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None: 	CORE_ASSERT(false, "Not a recognised api!");	return nullptr;
		case RendererAPI::Api::OpenGL:	CORE_ASSERT(false, "OpenGL is not a supported api!");	return nullptr;
		case RendererAPI::Api::Vulkan:	CORE_ASSERT(false, "Vulkan is not a supported api!");	return nullptr;
		case RendererAPI::Api::DX11:	CORE_ASSERT(false, "DirectX 11 is not a supported api!");	return nullptr;
		case RendererAPI::Api::DX12:	return CreateScope<D3D12Texture>(initData, width, height, depth, format);
		default:
			return nullptr;
		}
	}

	ScopePointer<Texture> Texture::Create
	(
		const void* initData,
		UINT32 width,
		UINT32 height,
		TextureFormat format
	)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None: 	CORE_ASSERT(false, "Not a recognised api!");	return nullptr;
		case RendererAPI::Api::OpenGL:	CORE_ASSERT(false, "OpenGL is not a supported api!");	return nullptr;
		case RendererAPI::Api::Vulkan:	CORE_ASSERT(false, "Vulkan is not a supported api!");	return nullptr;
		case RendererAPI::Api::DX11:	CORE_ASSERT(false, "DirectX 11 is not a supported api!");	return nullptr;
		case RendererAPI::Api::DX12:	return CreateScope<D3D12Texture>(initData, width, height, format);
		default:
			return nullptr;
		}
	}

	ScopePointer<Texture> Texture::Create(const std::wstring& fileName, const std::string& name)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None: 	CORE_ASSERT(false, "Not a recognised api!");	return nullptr;
		case RendererAPI::Api::OpenGL:	CORE_ASSERT(false, "OpenGL is not a supported api!");	return nullptr;
		case RendererAPI::Api::Vulkan:	CORE_ASSERT(false, "Vulkan is not a supported api!");	return nullptr;
		case RendererAPI::Api::DX11:	CORE_ASSERT(false, "DirectX 11 is not a supported api!");	return nullptr;
		case RendererAPI::Api::DX12:	return CreateScope<D3D12Texture>(fileName, name);
		default:
			return nullptr;
		}
	}

	void TextureLibrary::Add(const std::string& name, ScopePointer<Texture> texture)
	{
		CORE_ASSERT(!Exists(name), "Texture already exists!");
		Textures.emplace(name, std::move(texture));
	}

	void TextureLibrary::Remove(const std::string& name)
	{
		CORE_ASSERT(Exists(name), "Texture does not exist!");
		Textures.erase(name);
	}

	Texture* TextureLibrary::GetTexture(const std::string& name)
	{
		Texture* texture = Textures.at(name).get();
		return texture;
	}

	bool TextureLibrary::Exists(const std::string& name)
	{
		return (Textures.find(name) != Textures.end());
	}
}
