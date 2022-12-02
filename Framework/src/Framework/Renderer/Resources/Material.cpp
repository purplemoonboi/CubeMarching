#include "Framework/cmpch.h"
#include "Material.h"
#include "Framework/Renderer/Api/RendererAPI.h"

#include "Platform/DirectX12/DX12Material.h"

namespace Engine
{
	ScopePointer<Material> Material::Create(std::string&& name)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:	  CORE_ASSERT(false, "No Api found!"); return nullptr;
		case RendererAPI::Api::OpenGL:	  CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:	  CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:	  CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:	  return CreateScope<DX12Material>(std::move(name));
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	//Declare shader library.
	std::unordered_map<std::string, ScopePointer<Material>> MaterialLibrary::Materials;

	void MaterialLibrary::Add(const std::string& name, ScopePointer<Material> shader)
	{
		CORE_ASSERT(!Exists(name), "Material already exists!");
		Materials[name] = std::move(shader);
	}

	void MaterialLibrary::Add(ScopePointer<Material> shader)
	{
		auto& name = shader->GetName();
		Add(name, std::move(shader));
	}

	Material* MaterialLibrary::Get(const std::string& name)
	{
		CORE_ASSERT(Exists(name), "Material not found!");
		return Materials[name].get();
	}

	UINT32 MaterialLibrary::Size()
	{
		return static_cast<UINT32>(Materials.size());
	}

	bool MaterialLibrary::Exists(const std::string& name)
	{
		return (Materials.find(name) != Materials.end());
	}

}