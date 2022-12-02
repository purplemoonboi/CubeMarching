#pragma once
#include <vector>

#include "Framework/Renderer/Buffers/Buffer.h"
#include "Framework/Renderer/Pipeline/PipelineStateObject.h"


#include "Platform/DirectX12/DirectX12.h"

namespace Engine
{

	class Shader;
	class GraphicsContext;
	class DX12GraphicsContext;
	
	class DX12PipelineStateObject : public PipelineStateObject
	{
	public:

		DX12PipelineStateObject
		(
			GraphicsContext* graphicsContext,
			const std::string& vertexShader,
			const std::string& pixelShader,
			const BufferLayout& layout,
			FillMode fillMode
		);


		DX12PipelineStateObject
		(
			GraphicsContext* graphicsContext,
			Shader* vertexShader,
			Shader* pixelShader,
			const BufferLayout& layout,
			FillMode fillMode 
		);

		DX12PipelineStateObject
		(
			GraphicsContext* graphicsContext,
			Shader* computeShader
		);


		// @brief We don't want to copy a PSO, hence inform compiler not to generate a copy constructor
		DX12PipelineStateObject(const DX12PipelineStateObject&) = delete;
		// @brief Again, remove the copy assignment operator as we don't want any copies being generated.
		auto operator=(const DX12PipelineStateObject&) noexcept -> DX12PipelineStateObject& = delete;
		// @brief We don't want to move a PSO, it should be an lvalue.
		DX12PipelineStateObject(DX12PipelineStateObject&&) = delete;
		// @brief Similarly, inform the compiler to remove to move assignment operator
		auto operator=(const DX12PipelineStateObject&&) noexcept -> DX12PipelineStateObject&& = delete;


		~DX12PipelineStateObject() override;

		// @brief Returns a raw pointer to the pso.
		ID3D12PipelineState* GetPipelineState() const { return Pso.Get(); }




	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> Pso;

		DXGI_FORMAT BackBufferFormat;
		DXGI_FORMAT DepthBufferFormat;

	};
}


