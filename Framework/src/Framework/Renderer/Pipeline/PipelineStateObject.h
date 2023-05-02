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


		static ScopePointer<PipelineStateObject> CreateSkyPso
		(
			GraphicsContext* graphicsContext,
			Shader* vertexShader,
			Shader* pixelShader
		);

		static ScopePointer<PipelineStateObject> CreateShadowPso
		(
			GraphicsContext* graphicsContext,
			Shader* vertexShader,
			Shader* pixelShader,
			const BufferLayout& layout
		);


		static ScopePointer<PipelineStateObject> CreatePso
		(
			GraphicsContext* graphicsContext,
			Shader* vertexShader,
			Shader* pixelShader,
			const BufferLayout& layout,
			FillMode fillMode = FillMode::Opaque
		);


		static ScopePointer<PipelineStateObject> CreateComputePso
		(
			ComputeApi* computeContext,
			Shader* computeShader,
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature
		);
	};
}
