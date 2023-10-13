#pragma once
#include "Framework/Renderer/Shaders/Shader.h"
#include "Platform/DirectX12/DirectX12.h"

namespace Foundation::Graphics::D3D12
{
	class D3D12RenderPipeline;
	using Microsoft::WRL::ComPtr;

	class D3D12Shader : public Shader
	{
	public:
		D3D12Shader() = default;
		D3D12Shader(const ShaderArgs& args, EShaderType type);

		[[nodiscard]] const ComPtr<ID3DBlob>& GetComPointer() const { return ShaderData; }
		[[nodiscard]] ComPtr<ID3DBlob>& GetComPointer() { return ShaderData; }

	private:
		ComPtr<ID3DBlob> ShaderData;
	};


}
