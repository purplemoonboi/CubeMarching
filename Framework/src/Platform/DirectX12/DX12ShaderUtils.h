#pragma once
#include <string>

#include "DirectX12.h"

namespace Engine
{
	using Microsoft::WRL::ComPtr;

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

	private:

	};
}
