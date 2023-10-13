#include "Framework/cmpch.h"
#include "Framework/Core/Core.h"
#include "Framework/Core/Log/Log.h"
#include "Shader.h"
#include "Framework/Renderer/Api/RendererAPI.h"

#include "Platform/DirectX12/Shaders/D3D12Shader.h"


namespace Engine
{


	Shader* Shader::Create(const ShaderArgs args, ShaderType type)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:	  CORE_ASSERT(false, "No Api found!"); return nullptr;
		case RendererAPI::Api::OpenGL:	  CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:	  CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:	  CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:	  return CreateScope<D3D12Shader>(args, type);

		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	//Declare shader library.
	std::unordered_map<std::string, ScopePointer<Shader>> ShaderLibrary::Shaders;

	void ShaderLibrary::Add(const std::string& name, ScopePointer<Shader> shader)
	{
		CORE_ASSERT(!Exists(name), "Shader already exists!");
		Shaders[name] = std::move(shader);
	}

	void ShaderLibrary::Add(ScopePointer<Shader> shader)
	{
		auto& name = shader->GetName();
		Add(name, std::move(shader));
	}

	Shader* ShaderLibrary::Load(const std::string& filePath)
	{
		auto shader = Shader::Create(filePath);
		Shader* out = shader.get();
		Add(std::move(shader));
		return out;
	}

	Shader* ShaderLibrary::Load(const std::string& name, const std::wstring& filePath, std::string&& entryPoint, std::string&& target)
	{
		auto shader = Shader::Create(filePath, entryPoint, target);
		Shader* out = shader.get();
		Add(std::move(shader));
		return out;
	}

	Shader* ShaderLibrary::GetShader(const std::string& name)
	{
		CORE_ASSERT(Exists(name), "Shader not found!");
		return Shaders[name].get();
	}

	bool ShaderLibrary::Exists(const std::string& name)
	{
		return (Shaders.find(name) != Shaders.end());
	}




}

