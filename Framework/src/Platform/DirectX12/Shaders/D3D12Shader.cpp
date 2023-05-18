#include "D3D12Shader.h"
#include "Framework/Core/Log/Log.h"

namespace Foundation
{


	D3D12Shader::D3D12Shader(const std::wstring& filePath, const std::string& entryPoint, const std::string& target, D3D_SHADER_MACRO* defines)
		:
		FilePath(filePath)
	{
		BuildAndCompileShader(defines, entryPoint, target);
		if (target[0] == 'v')
			Type = ShaderType::VS;
		if (target[0] == 'h')
			Type = ShaderType::HS;
		if (target[0] == 'd')
			Type = ShaderType::DS;
		if (target[0] == 'g')
			Type = ShaderType::GS;
		if (target[0] == 'p')
			Type = ShaderType::PS;
	}

	D3D12Shader::D3D12Shader(std::wstring&& filePath, std::string&& entryPoint, std::string&& target, D3D_SHADER_MACRO* defines)
		:
		FilePath(std::move(filePath))
	{
		BuildAndCompileShader(defines, std::move(entryPoint), std::move(target));
		if (target[0] == 'v')
			Type = ShaderType::VS;
		if (target[0] == 'h')
			Type = ShaderType::HS;
		if (target[0] == 'd')
			Type = ShaderType::DS;
		if (target[0] == 'g')
			Type = ShaderType::GS;
		if (target[0] == 'p')
			Type = ShaderType::PS;
	}

	void D3D12Shader::BuildAndCompileShader
	(
		D3D_SHADER_MACRO* defines, 
		const std::string& entryPoint,
		const std::string& target
	)
	{
		ShaderData = CompileShader(FilePath, defines, entryPoint, target);
	}

	void D3D12Shader::Bind() const
	{

	}

	void D3D12Shader::UnBind() const
	{
	}

	void D3D12Shader::SetFloat(std::string&& name, float value)
	{
	}

	void D3D12Shader::SetFloat2(std::string&& name, const DirectX::XMFLOAT2& matrix)
	{
	}

	void D3D12Shader::SetFloat3(std::string&& name, const DirectX::XMFLOAT3& matrix)
	{
	}

	void D3D12Shader::SetFloat4(std::string&& name, const DirectX::XMFLOAT4& matrix)
	{
	}

	void D3D12Shader::SetMat3(std::string&& name, const DirectX::XMFLOAT3X3& matrix)
	{
	}

	void D3D12Shader::SetMat4(std::string&& name, const DirectX::XMFLOAT4X4& matrix)
	{
	}

	const std::string& D3D12Shader::GetName() const
	{
		return Name;
	}

	ComPtr<ID3DBlob> D3D12Shader::CompileShader
	(
		const std::wstring& fileName,
		const D3D_SHADER_MACRO* defines,
		const std::string& entryPoint,
		const std::string& target
	)
	{
		UINT compileFlags = 0;

#ifdef CM_DEBUG
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		HRESULT hr = S_OK;
		ComPtr<ID3DBlob> byteCode = nullptr;
		ComPtr<ID3DBlob> errors;

		hr = D3DCompileFromFile
		(
			fileName.c_str(),
			defines,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(),
			target.c_str(),
			compileFlags,
			0,
			&byteCode,
			&errors
		);

		if (errors != nullptr)
		{
			OutputDebugStringA(static_cast<char*>(errors->GetBufferPointer()));
		}

		THROW_ON_FAILURE(hr);

		return byteCode;

	}
}
