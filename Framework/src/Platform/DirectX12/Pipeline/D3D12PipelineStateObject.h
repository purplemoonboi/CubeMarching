#pragma once
#include <vector>

#include "Framework/Renderer/Buffers/Buffer.h"
#include "Framework/Renderer/Pipeline/PipelineStateObject.h"


#include "Platform/DirectX12/DirectX12.h"

namespace Engine
{
	class D3D12Shader;
	class ComputeApi;

	
	class D3D12Context;

	using Microsoft::WRL::ComPtr;

	
	
	class D3D12PipelineStateObject : public PipelineStateObject
	{
	public:

		D3D12PipelineStateObject
		(
			D3D12Context* graphicsContext,
			const PipelineResourceDesc& args,
			const PipelineDesc& desc
		);

		D3D12PipelineStateObject
		(
			D3D12Context* graphicsContext,
			D3D12Shader* vertexShader,
			D3D12Shader* pixelShader,
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

		std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();


	private:
		void InitialiseRoot(ID3D12Device* device, const PipelineResourceDesc& desc);

		ComPtr<ID3D12PipelineState> Pso;
		ComPtr<ID3D12RootSignature> RootSignature;

		DXGI_FORMAT BackBufferFormat;
		DXGI_FORMAT DepthBufferFormat;

		// Array of shaders to define the pipeline
		std::array<ScopePointer<D3D12Shader*>, 5> ShaderPipeline = { nullptr };
	};
}


