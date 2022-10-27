#include "Framework/cmpch.h"
#include "Framework/Core/Core.h"
#include "Framework/Core/Log/Log.h"
#include "Shader.h"
#include "Renderer.h"

#include "Platform/DirectX12/DX12Shader.h"

namespace DX12Framework
{

	RefPointer<Shader> Shader::Create(const std::wstring& filePath)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:	  CORE_ASSERT(false, "No Api found!"); return nullptr;
		case RendererAPI::Api::OpenGL:	  CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:	  CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:	  CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:	  return CreateRef<DX12Shader>(filePath);
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	RefPointer<Shader> Shader::Create(const std::wstring& filePath, const std::wstring& vertexSrc, const std::wstring& fragmentSrc)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:	  CORE_ASSERT(false, "No Api found!"); return nullptr;
		case RendererAPI::Api::OpenGL:	  CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:	  CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:	  CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:	  return CreateRef<DX12Shader>(filePath);

		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	void ShaderLibrary::Add(const std::wstring& name, const RefPointer<Shader>& shader)
	{
		CORE_ASSERT(!Exists(name), "Shader already exists!");
		Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const RefPointer<Shader>& shader)
	{
		auto& name = shader->GetName();
		Add(name, shader);
	}

	RefPointer<Shader> ShaderLibrary::Load(const std::wstring& filePath)
	{
		auto shader = Shader::Create(filePath);
		Add(shader);
		return shader;
	}

	RefPointer<Shader> ShaderLibrary::Load(const std::wstring& name, const std::wstring& filePath)
	{
		auto shader = Shader::Create(filePath);
		Add(shader);
		return shader;
	}

	RefPointer<Shader> ShaderLibrary::Get(const std::wstring& name)
	{
		CORE_ASSERT(Exists(name), "Shader not found!");
		return Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::wstring& name) const
	{
		return (Shaders.find(name) != Shaders.end());
	} 

}

