#include "Framework/cmpch.h"
#include "Framework/Core/Core.h"
#include "Framework/Core/Log/Log.h"
#include "Shader.h"
#include "Framework/Renderer/Api/RendererAPI.h"

#include "Platform/DirectX12/Shaders/D3D12Shader.h"


namespace Foundation::Graphics
{

	ScopePointer<Shader> Shader::Create(const std::string& filePath)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:	  CORE_ASSERT(false, "No Api found!"); return nullptr;
		case RendererAPI::Api::OpenGL:	  CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:	  CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:	  CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:	  return CreateScope<D3D12::D3D12Shader>(filePath);
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	ScopePointer<Shader> Shader::Create(std::string&& filepath)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:	  CORE_ASSERT(false, "No Api found!"); return nullptr;
		case RendererAPI::Api::OpenGL:	  CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:	  CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:	  CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:	  return CreateScope<D3D12::D3D12Shader>(std::move(filepath));

		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}



	ScopePointer<Shader> Shader::Create(const ShaderDescription& shaderDesc)
	{
	}

	ScopePointer<Shader> Shader::Create(ShaderDescription&& shaderDesc)
	{
	}

	ScopePointer<Shader> Shader::Create(const std::wstring& filePath, const std::string& entryPoint, const std::string& target)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:	  CORE_ASSERT(false, "No Api found!"); return nullptr;
		case RendererAPI::Api::OpenGL:	  CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:	  CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:	  CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:	  return CreateScope<D3D12::D3D12Shader>(filePath, entryPoint, target);

		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	ScopePointer<Shader> Shader::Create(std::wstring&& filePath, std::string&& entryPoint, std::string&& target)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:	  CORE_ASSERT(false, "No Api found!"); return nullptr;
		case RendererAPI::Api::OpenGL:	  CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:	  CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:	  CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:	  return CreateScope<D3D12::D3D12Shader>(std::move(filePath), std::move(entryPoint), std::move(target));

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

