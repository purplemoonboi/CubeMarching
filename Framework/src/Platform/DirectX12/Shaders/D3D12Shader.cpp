#include "D3D12Shader.h"
#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Pipeline/D3D12RenderPipeline.h"
#include "Framework/Renderer/Shaders/Shader.h

namespace Foundation::Graphics::D3D12
{
	inline ComPtr<ID3DBlob> CompileShader
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


	D3D12Shader::D3D12Shader
	(
		const ShaderArgs& args,
		EShaderType type
	)
		:
		Shader(entryPoint, type)
		, ShaderData(CompileShader(args.FilePath, args.macros, args.EntryPoint, args.ShaderModel))
	{}
}
