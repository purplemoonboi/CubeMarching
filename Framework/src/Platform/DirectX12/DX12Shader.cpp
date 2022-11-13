#include "DX12Shader.h"
#include "Framework/Core/Log/Log.h"

namespace Engine
{


	DX12Shader::DX12Shader(const std::wstring& filePath, const std::string& entryPoint, const std::string& target, D3D_SHADER_MACRO* defines)
		:
		FilePath(filePath)
	{
		BuildAndCompileShader(defines, entryPoint, target);
	}

	DX12Shader::DX12Shader(std::wstring&& filePath, std::string&& entryPoint, std::string&& target, D3D_SHADER_MACRO* defines)
		:
		FilePath(std::move(filePath))
	{
		BuildAndCompileShader(defines, std::move(entryPoint), std::move(target));
	}

	void DX12Shader::BuildAndCompileShader
	(
		D3D_SHADER_MACRO* defines, 
		const std::string& entryPoint,
		const std::string& target
	)
	{
		ShaderData = CompileShader(FilePath, defines, entryPoint, target);
	}

	void DX12Shader::Bind() const
	{

	}

	void DX12Shader::UnBind() const
	{
	}

	void DX12Shader::SetFloat(std::string&& name, float value)
	{
	}

	void DX12Shader::SetFloat2(std::string&& name, const DirectX::XMFLOAT2& matrix)
	{
	}

	void DX12Shader::SetFloat3(std::string&& name, const DirectX::XMFLOAT3& matrix)
	{
	}

	void DX12Shader::SetFloat4(std::string&& name, const DirectX::XMFLOAT4& matrix)
	{
	}

	void DX12Shader::SetMat3(std::string&& name, const DirectX::XMFLOAT3X3& matrix)
	{
	}

	void DX12Shader::SetMat4(std::string&& name, const DirectX::XMFLOAT4X4& matrix)
	{



	}

	const std::string& DX12Shader::GetName() const
	{
		return Name;
	}


	ComPtr<ID3DBlob> DX12Shader::CompileShader
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
			OutputDebugStringA((char*)(errors->GetBufferPointer()));
		}

		THROW_ON_FAILURE(hr);

		return byteCode;

	}
}
