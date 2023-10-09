#pragma once
#include "Framework/Core/core.h"

#include <string>
#include <unordered_map>

namespace Foundation::Graphics
{

	enum class EShaderType
	{
		VS = 0,
		PS = 0,
		HS = 0,
		DS = 0,
		GS = 0,
		AS = 0,
		MS = 0,
		RS = 0,
	};

	struct ShaderDesc
	{
		std::wstring	FilePath;
		std::string		Name;
		std::string		Target;
		std::string		EntryPoint;
		EShaderType		Type;
		void*			ShaderMacros;
	};

	// @brief The shader class is a high level object which allows the user to allocate
	// data onto the GPU.
	// Needs implemented per API
	class Shader
	{
	public:
		Shader(const std::string_view& name, EShaderType type);
		virtual ~Shader() = default;

		[[nodiscard]] virtual EShaderType GetShaderType() const { return Type; };
		[[nodiscard]] virtual const std::string& GetName() const { return Name; };

		static RefPointer<Shader> Create(const std::wstring& filePath, const std::string& entryPoint, const std::string& target, EShaderType type, void* macros = nullptr);

	private:
		std::string Name;
		EShaderType Type;
		void* ByteCode;
	};


	// @Brief Stores pointers to shader object using an unordered map.
	class ShaderLibrary
	{
	public:

		static void Add(const RefPointer<Shader>& shader);

		static void Add(const std::string& name, const RefPointer<Shader>& shader);

		static RefPointer<Shader> Load(const ShaderDesc& desc);

		static RefPointer<Shader> Get(const std::string& name);

		[[nodiscard]] static bool Exists(const std::string& name) ;

	private:
		static std::unordered_map<std::string, RefPointer<Shader>> ShaderLib;
	};
}
