#include "Texture.h"
#include "Framework/Renderer/Api/RendererAPI.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"

namespace Foundation::Graphics
{
	using namespace D3D12;
	//using namespace Vulkan;

	std::unordered_map<ResourceFormat, UINT64> TextureTypeSize::TypeSize = {};

	ScopePointer<Texture> Texture::Create
	(
		const void* initData,
		UINT32 width,
		UINT32 height,
		UINT16 depth,
		ResourceFormat format 
	)
	{
		switch(RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None: 	CORE_ASSERT(false, "Not a recognised api!");	return nullptr;
		case RendererAPI::Api::Vulkan:	CORE_ASSERT(false, "Vulkan is not a supported api!");	return nullptr;
		case RendererAPI::Api::DX12:	return CreateScope<D3D12Texture>(initData, width, height, depth, format);
		default:
			return nullptr;
		}
	}

	ScopePointer<Texture> Texture::Create(const std::wstring& fileName, const std::string_view& name)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None: 	CORE_ASSERT(false, "Not a recognised api!");	return nullptr;
		case RendererAPI::Api::Vulkan:	CORE_ASSERT(false, "Vulkan is not a supported api!");	return nullptr;
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
		
		//TODO: Add to the deferred resource vector

		Textures.erase(name);
	}

	Texture* TextureLibrary::GetTexture(const std::string_view& name)
	{
		Texture* texture = Textures.at(std::string(name)).get();
		return texture;
	}

	bool TextureLibrary::Exists(const std::string_view& name)
	{
		return (Textures.find(std::string(name)) != Textures.end());
	}
}
