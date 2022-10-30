#include "DX12ShaderUtils.h"

#include "Framework/Core/Log/Log.h"


namespace DX12Framework
{

	ComPtr<ID3DBlob> DX12ShaderUtils::CompileShader
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
			OutputDebugStringA((char*)errors->GetBufferPointer());
		}

		THROW_ON_FAILURE(hr);

		return byteCode;

	}
}
