#include "Framework/cmpch.h"
#include "Framework/Core/Core.h"
#include "Framework/Core/Log/Log.h"
#include "Shader.h"
#include "Framework/Renderer/Api/RendererAPI.h"

#include "Platform/DirectX12/DX12Shader.h"


namespace Engine
{

	ScopePointer<Shader> Shader::Create(const std::string& filePath)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:	  CORE_ASSERT(false, "No Api found!"); return nullptr;
		case RendererAPI::Api::OpenGL:	  CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:	  CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:	  CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:	  return CreateScope<DX12Shader>(filePath);
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
		case RendererAPI::Api::DX12:	  return CreateScope<DX12Shader>(std::move(filepath));

		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	ScopePointer<Shader> Shader::Create(const std::wstring& filePath, const std::string& entryPoint, const std::string& target, D3D_SHADER_MACRO* defines)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:	  CORE_ASSERT(false, "No Api found!"); return nullptr;
		case RendererAPI::Api::OpenGL:	  CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:	  CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:	  CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:	  return CreateScope<DX12Shader>(filePath, entryPoint, target, defines);

		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	ScopePointer<Shader> Shader::Create(std::wstring&& filePath, std::string&& entryPoint, std::string&& target, D3D_SHADER_MACRO* defines)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:	  CORE_ASSERT(false, "No Api found!"); return nullptr;
		case RendererAPI::Api::OpenGL:	  CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:	  CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:	  CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:	  return CreateScope<DX12Shader>(std::move(filePath), std::move(entryPoint), std::move(target), defines);

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

	ScopePointer<Shader> ShaderLibrary::Load(const std::string& filePath)
	{
		auto shader = Shader::Create(filePath);
		Add(std::move(shader));
		return shader;
	}

	//TODO: Need to fix this!
	//TODO: Can't return pointer after we have moved data!
	ScopePointer<Shader> ShaderLibrary::Load(const std::string& name, const std::wstring& filePath, std::string&& entryPoint, std::string&& target)
	{
		auto shader = Shader::Create(filePath, entryPoint, target);
		Add(std::move(shader));
		return nullptr;
	}

	Shader* ShaderLibrary::Get(const std::string& name)
	{
		CORE_ASSERT(Exists(name), "Shader not found!");
		return Shaders[name].get();
	}

	bool ShaderLibrary::Exists(const std::string& name)
	{
		return (Shaders.find(name) != Shaders.end());
	}




}

