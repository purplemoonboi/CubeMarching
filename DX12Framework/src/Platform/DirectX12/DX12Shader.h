#pragma once
#include "Framework/Renderer/Shader.h"
#include "DirectX12.h"

namespace DX12Framework
{
	using Microsoft::WRL::ComPtr;

	class DX12Shader : public Shader
	{
		DX12Shader(const std::wstring& fileName);
	public:

		
	private:

		std::wstring FileName;

	};

	class DX12ShaderUtils
	{
	public:

		static ComPtr<ID3DBlob> CompileShader
		(
			const std::wstring& fileName, 
			const D3D_SHADER_MACRO* defines,
			const std::string& entryPoint,
			const std::string& target
		);


	};
}
