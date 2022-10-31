#include "DX12Shader.h"

#include "Framework/Core/Log/Log.h"

namespace Engine
{


	DX12Shader::DX12Shader(const std::wstring& fileName)
		:
		FileName(fileName)
	{
	}

	void DX12Shader::Bind() const
	{
	}

	void DX12Shader::UnBind() const
	{
	}

	const std::wstring& DX12Shader::GetName() const
	{
		return std::wstring();
	}
}
