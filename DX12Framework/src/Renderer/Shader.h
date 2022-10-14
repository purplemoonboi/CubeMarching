#pragma once
#include "Core/core.h"






namespace DX12Framework
{

	// @brief The shader class is a high level object which allows the user to allocate
	// data onto the GPU.
	// Needs implemented per API
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;

		virtual void UnBind() const = 0;




		virtual const std::wstring& GetName() const = 0;

		static RefPointer<Shader> Create(const std::wstring& filepath);

		static RefPointer<Shader> Create(const std::wstring& name, const std::wstring& vertexSrc, const std::wstring& fragmentSrc);
	};


	// @Brief Stores pointers to shader object using an unordered map.
	class ShaderLibrary
	{
	public:

		void Add(const RefPointer<Shader>& shader);

		void Add(const std::wstring& name, const RefPointer<Shader>& shader);

		RefPointer<Shader> Load(const std::wstring& filePath);

		RefPointer<Shader> Load(const std::wstring& name, const std::wstring& filePath);

		RefPointer<Shader> Get(const std::wstring& name);

		bool Exists(const std::wstring& name) const;

	private:
		std::unordered_map<std::wstring, RefPointer<Shader>> Shaders;
	};
}
