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

	// @brief The shader class is a high level object which allows the user to allocate
	// data onto the GPU.
	// Needs implemented per Api
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;

		virtual void UnBind() const = 0;

		virtual void SetFloat(std::string&& name, float value) = 0;
		virtual void SetFloat2(std::string&& name, const DirectX::XMFLOAT2& matrix) = 0;
		virtual void SetFloat3(std::string&& name, const DirectX::XMFLOAT3& matrix) = 0;
		virtual void SetFloat4(std::string&& name, const DirectX::XMFLOAT4& matrix) = 0;

		virtual void SetMat3(std::string&& name, const DirectX::XMFLOAT3X3& matrix) = 0;
		virtual void SetMat4(std::string&& name, const DirectX::XMFLOAT4X4& matrix) = 0;


		virtual const std::string& GetName() const = 0;

		static RefPointer<Shader> Create(const std::string& filepath);

		static RefPointer<Shader> Create(std::string&& filepath);

		static RefPointer<Shader> Create(const std::wstring& filePath, const std::string& entryPoint, const std::string& target, D3D_SHADER_MACRO* defines = nullptr);

		static RefPointer<Shader> Create(std::wstring&& filePath, std::string&& entryPoint, std::string&& target, D3D_SHADER_MACRO* defines = nullptr);
	};


	//// @Brief Stores pointers to shader object using an unordered map.
	//class ShaderLibrary
	//{
	//public:

	//	static void Add(const RefPointer<Shader>& shader);
	//	
	//	static void Add(const std::string& name, const RefPointer<Shader>& shader);
	//	
	//	static RefPointer<Shader> Load(const std::string& filePath);
	//	
	//	static RefPointer<Shader> Load(const std::string& name, const std::wstring& filePath, std::string&& entryPoint, std::string&& target);
	//	
	//	static RefPointer<Shader> Get(const std::string& name);
	//	
	//	static bool Exists(const std::string& name);

	//private:
	//	static std::unordered_map<std::string, RefPointer<Shader>> Shaders;
	//};


}
