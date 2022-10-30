#pragma once
#include "Framework/Renderer/Shader.h"
#include "DirectX12.h"

namespace DX12Framework
{
	using Microsoft::WRL::ComPtr;

	class DX12Shader : public Shader
	{
	public:
		DX12Shader(const std::wstring& fileName);

		virtual void Bind() const override;

		virtual void UnBind() const override;



		virtual const std::wstring& GetName() const override;
		
	private:

		std::wstring FileName;

	};


}
