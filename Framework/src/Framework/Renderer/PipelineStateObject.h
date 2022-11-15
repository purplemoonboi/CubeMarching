#pragma once
#include "Framework/Core/core.h"

namespace Engine
{
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


		static RefPointer<PipelineStateObject> Create
		(
			GraphicsContext* graphicsContext,
			const std::string& vertexShader,
			const std::string& pixelShader,
			const BufferLayout& layout,
			FillMode fillMode = FillMode::Opaque
		);


		static RefPointer<PipelineStateObject> Create
		(
			GraphicsContext* graphicsContext,
			Shader* vertexShader,
			Shader* pixelShader,
			const BufferLayout& layout,
			FillMode fillMode = FillMode::Opaque
		);
	};
}
