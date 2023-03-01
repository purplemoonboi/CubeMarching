#pragma once
#include <wrl/client.h>
#include "Framework/Core/core.h"


#include <Platform/DirectX12/DirectX12.h>

namespace Engine
{
	class ComputeApi;
	class BufferLayout;
	class Shader;
	class GraphicsContext;

	enum class FillMode
	{
		WireFrame = 2,
		Opaque = 3,
	};

	// @brief Base class for describing the rendering pipeline.
	class PipelineStateObject 
	{
	public:
		PipelineStateObject() = default;
		virtual ~PipelineStateObject() = default;


		static ScopePointer<PipelineStateObject> Create
		(
			GraphicsContext* graphicsContext,
			const std::string& vertexShader,
			const std::string& pixelShader,
			const BufferLayout& layout,
			FillMode fillMode = FillMode::Opaque
		);


		static ScopePointer<PipelineStateObject> Create
		(
			GraphicsContext* graphicsContext,
			Shader* vertexShader,
			Shader* pixelShader,
			const BufferLayout& layout,
			FillMode fillMode = FillMode::Opaque
		);


		static ScopePointer<PipelineStateObject> Create
		(
			ComputeApi* computeContext,
			Shader* computeShader,
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature
		);
	};
}
