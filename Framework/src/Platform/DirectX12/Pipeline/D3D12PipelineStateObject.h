#pragma once
#include <Framework/Renderer/Buffers/Buffer.h>
#include <Framework/Renderer/Pipeline/PipelineStateObject.h>
#include "Platform/DirectX12/DirectX12.h"

namespace Foundation::Graphics::D3D12
{
	class D3D12Shader;
	class D3D12Context;

	class D3D12PipelineStateObject : public PipelineStateObject
	{
	public:

		D3D12PipelineStateObject
		(
			PipelineDesc& desc
		);

		D3D12PipelineStateObject
		(
			D3D12Shader* vertexShader,
			D3D12Shader* pixelShader,
			FillMode fillMode 
		);

		/*D3D12PipelineStateObject
		(
			Shader* computeShader,
			ComPtr<ID3D12RootSignature> rootSignature
		);*/

		DISABLE_COPY_AND_MOVE(D3D12PipelineStateObject);


		~D3D12PipelineStateObject() override;

		// @brief Returns a raw pointer to the pso.
		[[nodiscard]] ID3D12PipelineState* GetPipelineState() const { return Pso.Get(); }

		[[nodiscard]] ComPtr<ID3D12PipelineState> GetComPtr() const { return Pso; }



	private:
		void InitialiseRoot(const PipelineInputDesc& pipelineIn);

		ComPtr<ID3D12PipelineState> Pso;
		ComPtr<ID3D12RootSignature> RootSignature;

		DXGI_FORMAT BackBufferFormat;
		DXGI_FORMAT DepthBufferFormat;

		// Array of shaders to define the pipeline
		std::array<ScopePointer<D3D12Shader*>, 5> ShaderPipeline = { nullptr };
	};
}


