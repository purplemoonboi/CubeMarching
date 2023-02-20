#pragma once
#include <vector>

#include "Framework/Renderer/Buffers/Buffer.h"
#include "Framework/Renderer/Pipeline/PipelineStateObject.h"


#include "Platform/DirectX12/DirectX12.h"

namespace Engine
{
	class ComputeApi;

	class Shader;
	class GraphicsContext;
	class D3D12Context;

	using Microsoft::WRL::ComPtr;
	
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
			ComputeApi* computeContext,
			Shader* computeShader,
			ComPtr<ID3D12RootSignature> rootSignature
		);


		D3D12PipelineStateObject(const D3D12PipelineStateObject&) = delete;
		auto operator=(const D3D12PipelineStateObject&) noexcept -> D3D12PipelineStateObject& = delete;
		D3D12PipelineStateObject(D3D12PipelineStateObject&&) = delete;
		auto operator=(const D3D12PipelineStateObject&&) noexcept -> D3D12PipelineStateObject&& = delete;


		~D3D12PipelineStateObject() override;

		// @brief Returns a raw pointer to the pso.
		[[nodiscard]] ID3D12PipelineState* GetPipelineState() const { return Pso.Get(); }

		[[nodiscard]] ComPtr<ID3D12PipelineState> GetComPtr() const { return Pso; }

	private:
		ComPtr<ID3D12PipelineState> Pso;

		DXGI_FORMAT BackBufferFormat;
		DXGI_FORMAT DepthBufferFormat;

	};
}


