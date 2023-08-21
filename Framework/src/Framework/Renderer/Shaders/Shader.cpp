#include "Shader.h"

#include "Framework/Renderer/Renderer3D/Renderer.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"

namespace Foundation::Graphics
{
	Shader::Shader(const std::string_view& name, EShaderType type)
		:
			Name(name)
		,	Type(type)
	{}

	RefPointer<Shader> Shader::Create(const std::wstring& filePath, const std::string& entryPoint, const std::string& target, EShaderType type, void* macros)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:	  CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::Api::OpenGL:	  CORE_ASSERT(false, "OpenGL 4.5 is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:	  CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:	  CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:	  CreateRef<D3D12::D3D12Shader>(filePath, entryPoint, target, type, static_cast<D3D_SHADER_MACRO*>(macros));
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	std::unordered_map<std::string, RefPointer<Shader>> ShaderLibrary::ShaderLib;

	void ShaderLibrary::Add(const std::string& name, const RefPointer<Shader>& shader)
	{
		CORE_ASSERT(!Exists(name), "Shader already exists!");
		ShaderLib[name] = shader;
	}

	void ShaderLibrary::Add(const RefPointer<Shader>& shader)
	{
		auto& name = shader->GetName();
		Add(name, shader);
	}

	RefPointer<Shader> ShaderLibrary::Load(const ShaderDesc& desc)
	{
		auto shader = Shader::Create(desc.FilePath, desc.EntryPoint, desc.Target, desc.Type, desc.ShaderMacros);
		Add(shader);
		return shader;
	}

	RefPointer<Shader> ShaderLibrary::Get(const std::string& name)
	{
		CORE_ASSERT(Exists(name), "Shader not found!");
		return ShaderLib[name];
	}

	bool ShaderLibrary::Exists(const std::string& name) 
	{
		return ShaderLib.count(name) > 0;
	} 

}

