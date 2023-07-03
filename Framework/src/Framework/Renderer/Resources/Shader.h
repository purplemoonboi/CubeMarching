#pragma once
#include <string>
#include <unordered_map>

#include "Framework/Core/core.h"

namespace Foundation::Graphics
{
	class RenderPipeline;

	enum class ShaderType
	{
		VS = 0,
		HS,
		DS,
		GS,
		PS
	};

#define VERTEX_SHADER 0
#define HULL_SHADER 1
#define DOMAIN_SHADER 2
#define GEOMETRY_SHADER 3
#define PIXEL_SHADER 4
#define COMPUTE_SHADER 5
#define TASK_SHADER 6
#define MESH_SHADER 7

	typedef std::string ShaderMacro;

	struct ShaderDescription
	{
		explicit ShaderDescription(const std::string& name, const std::string& filepath, const std::string& entry, char target[8], const ShaderMacro& flags);

		std::string Name{};
		std::string FilePath{};
		std::string Entry{};
		char Target[8] {};
		ShaderMacro Macros{"DEFAULT"};
	};

	// @brief The shader class acts as an interface for shader code, ultimately stored in
	// pipeline object.
	class Shader
	{
	public:
		virtual ~Shader() = default;

		[[nodiscard]] virtual const std::string& GetName() const = 0;
		[[nodiscard]] virtual ShaderType GetShaderType() const = 0;

		static ScopePointer<Shader> Create(const std::string& filepath);
		static ScopePointer<Shader> Create(std::string&& filepath);
		static ScopePointer<Shader> Create(const std::wstring& filePath, const std::string& entryPoint, const std::string& target);
		static ScopePointer<Shader> Create(std::wstring&& filePath, std::string&& entryPoint, std::string&& target);
		static ScopePointer<Shader> Create(const ShaderDescription& shaderDesc);
		static ScopePointer<Shader> Create(ShaderDescription&& shaderDesc);
	};


	// @Brief Stores pointers to shader object using an unordered map.
	class ShaderLibrary
	{
	public:

		static void Add(ScopePointer<Shader> shader);
		
		static void Add(const std::string& name, ScopePointer<Shader> shader);
		
		static Shader* Load(const std::string& filePath);
		
		static Shader* Load(const std::string& name, const std::wstring& filePath, std::string&& entryPoint, std::string&& target);
		
		static Shader* GetShader(const std::string& name);
		
		static bool Exists(const std::string& name);

	private:
		static std::unordered_map<std::string, ScopePointer<Shader>> Shaders;
	};


}
