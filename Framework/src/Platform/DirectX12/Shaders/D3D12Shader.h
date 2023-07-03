#pragma once
#include "Framework/Renderer/Resources/Shader.h"
#include "Platform/DirectX12/DirectX12.h"

namespace Foundation::Graphics::D3D12
{
	class D3D12RenderPipeline;
	using Microsoft::WRL::ComPtr;

	class D3D12Shader : public Shader
	{
	public:
		D3D12Shader() = default;
		D3D12Shader(const std::wstring& filePath, const std::string& entryPoint, const std::string& target, D3D_SHADER_MACRO* defines = nullptr);
		D3D12Shader(std::wstring&& filePath, std::string&& entryPoint, std::string&& target, D3D_SHADER_MACRO* defines = nullptr);

		D3D12Shader(const std::string& fileName) {};
		D3D12Shader(std::string&& fileName) {};


		void BuildAndCompileShader(D3D_SHADER_MACRO* defines, const std::string& entryPoint, const std::string& target);


		// @brief Returns a com ptr object to the shader data.
		const ComPtr<ID3DBlob>& GetComPointer() const { return ShaderData; }

		ShaderType GetShaderType() const override { return Type; }

		// @brief Returns a raw pointer to the shader data.
		ID3DBlob* GetShader() const { return ShaderData.Get(); }

		const std::string& GetName() const override;
		
	private:
		ShaderType Type{ShaderType::VS};

		std::string Name{""};

		std::wstring FilePath{L""};

		ComPtr<ID3DBlob> ShaderData{ nullptr };

	};


}
