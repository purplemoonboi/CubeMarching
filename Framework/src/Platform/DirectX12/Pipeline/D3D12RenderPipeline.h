#pragma once
#include <Framework/Renderer/Buffers/Buffer.h>
#include <Framework/Renderer/Pipeline/RenderPipeline.h>
#include "Platform/DirectX12/DirectX12.h"

namespace Foundation::Graphics::D3D12
{
	class D3D12Shader;
	class D3D12Context;

	struct PipelineDeferredData
	{
		BYTE* Data{ nullptr };
		UINT64 MaxSizeInBytes{ 128U * 64U * 4U };
	};

	class D3D12RenderPipeline : public RenderPipeline
	{
	public:
		D3D12RenderPipeline() = default;
		D3D12RenderPipeline(PipelineDesc& desc);
		DISABLE_COPY(D3D12RenderPipeline);

		D3D12RenderPipeline(D3D12RenderPipeline&& desc) noexcept;
		auto operator=(D3D12RenderPipeline&& rhs) noexcept -> D3D12RenderPipeline&;


		~D3D12RenderPipeline()	override;
		void Bind()				override;
		void Destroy()			override;

		void SetVertexShader(const std::string_view& name,		Shader* shader)		override;
		void SetPixelShader(const std::string_view& name,		Shader* shader)		override;
		void SetHullShader(const std::string_view& name,		Shader* shader)		override;
		void SetDomainShader(const std::string_view& name,		Shader* shader)		override;
		void SetGeometryShader(const std::string_view& name,	Shader* shader)		override;


	public://D3D12
		[[nodiscard("")]] ID3D12PipelineState* GetPipelineState() const { return Pso.Get(); }
		[[nodiscard("")]] ID3D12RootSignature* GetPipelineRootSignature() const { return RootSignature.Get(); }
		[[nodiscard("")]] ComPtr<ID3D12PipelineState> GetComPtr() const { return Pso; }


	private:
		void InitialiseRoot(const std::vector<PipelineInputDesc>& pipelineIn);

		void LoadShader(const ShaderDesc& desc);

		ComPtr<ID3D12PipelineState> Pso{nullptr};
		ComPtr<ID3D12RootSignature> RootSignature{nullptr};

		DXGI_FORMAT BackBufferFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };
		DXGI_FORMAT DepthBufferFormat{ DXGI_FORMAT_D24_UNORM_S8_UINT };

		
	};
}


