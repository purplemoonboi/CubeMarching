#pragma once
#include <string>
#include <unordered_map>

#include "Framework/Core/core.h"

#ifndef  OPENGL_API 
#include <d3dcommon.h>
#include "DirectXMath.h"
#endif


namespace Engine
{

	enum class ShaderType
	{
		VS = 0,
		HS,
		DS,
		GS,
		PS
	};

	struct ShaderArgs
	{
		std::wstring FilePath;
		std::string EntryPoint;
		std::string ShaderModel{ "6.0" };
		void* macros{ nullptr };
	};

	// @brief The shader class is a high level object which allows the user to allocate
	// data onto the GPU.
	// Needs implemented per Api
	class Shader
	{
	public:
		Shader() = default;

		static Shader* Create(const ShaderArgs args, ShaderType type);

		ShaderArgs GetShaderArgs() const { return Args; }
		ShaderType GetShaderType() const { return Type; }

	protected:
		ShaderArgs Args;
		ShaderType Type;

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
