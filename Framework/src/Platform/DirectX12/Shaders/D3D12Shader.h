#pragma once
#include "Framework/Renderer/Resources/Shader.h"
#include "Platform/DirectX12/DirectX12.h"

namespace Foundation::Graphics::D3D12
{
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

		void Bind() const override;

		void UnBind() const override;

		void SetFloat(std::string&& name, float value) override;
		void SetFloat2(std::string&& name, const DirectX::XMFLOAT2& matrix) override;
		void SetFloat3(std::string&& name, const DirectX::XMFLOAT3& matrix) override;
		void SetFloat4(std::string&& name, const DirectX::XMFLOAT4& matrix) override;

		void SetMat3(std::string&& name, const DirectX::XMFLOAT3X3& matrix) override;
		void SetMat4(std::string&& name, const DirectX::XMFLOAT4X4& matrix) override;

		// @brief Returns a com ptr object to the shader data.
		const ComPtr<ID3DBlob>& GetComPointer() const { return ShaderData; }

		ShaderType GetShaderType() const override { return Type; }

		// @brief Returns a raw pointer to the shader data.
		ID3DBlob* GetShader() const { return ShaderData.Get(); }

		const std::string& GetName() const override;
		
	private:
		ShaderType Type;
		// @brief name of the shader, can be used to access from shader library
		std::string Name;

		std::wstring FilePath;

		// @brief Shader data in bytes
		ComPtr<ID3DBlob> ShaderData;


	private:

		// @brief Builds and compiles a shader
		static ComPtr<ID3DBlob> CompileShader
		(
			const std::wstring& fileName,
			const D3D_SHADER_MACRO* defines,
			const std::string& entryPoint,
			const std::string& target
		);

	};


}
