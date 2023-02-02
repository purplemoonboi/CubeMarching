#pragma once
#include <vector>

#include "Framework/Renderer/Buffers/Buffer.h"
#include "Framework/Renderer/Pipeline/PipelineStateObject.h"


#include "Platform/DirectX12/DirectX12.h"

namespace Engine
{

	class Shader;
	class GraphicsContext;
	class D3D12Context;
	
	class D3D12PipelineStateObject : public PipelineStateObject
	{
	public:

		D3D12PipelineStateObject
		(
			GraphicsContext* graphicsContext,
			const std::string& vertexShader,
			const std::string& pixelShader,
			const BufferLayout& layout,
			FillMode fillMode
		);


		D3D12PipelineStateObject
		(
			GraphicsContext* graphicsContext,
			Shader* vertexShader,
			Shader* pixelShader,
			const BufferLayout& layout,
			FillMode fillMode 
		);

		D3D12PipelineStateObject
		(
			GraphicsContext* graphicsContext,
			Shader* computeShader
		);


		// @brief We don't want to copy a PSO, hence inform compiler not to generate a copy constructor
		D3D12PipelineStateObject(const D3D12PipelineStateObject&) = delete;
		// @brief Again, remove the copy assignment operator as we don't want any copies being generated.
		auto operator=(const D3D12PipelineStateObject&) noexcept -> D3D12PipelineStateObject& = delete;
		// @brief We don't want to move a PSO, it should be an lvalue.
		D3D12PipelineStateObject(D3D12PipelineStateObject&&) = delete;
		// @brief Similarly, inform the compiler to remove to move assignment operator
		auto operator=(const D3D12PipelineStateObject&&) noexcept -> D3D12PipelineStateObject&& = delete;


		~D3D12PipelineStateObject() override;

		// @brief Returns a raw pointer to the pso.
		ID3D12PipelineState* GetPipelineState() const { return Pso.Get(); }




	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> Pso;

		DXGI_FORMAT BackBufferFormat;
		DXGI_FORMAT DepthBufferFormat;

	};
}


