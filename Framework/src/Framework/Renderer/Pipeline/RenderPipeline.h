#pragma once
#include <wrl/client.h>
#include "Framework/Core/core.h"
#include "Framework/Renderer/Buffers/Buffer.h"
#include "Framework/Renderer/Shaders/Shader.h"

namespace Foundation::Graphics
{

	enum class FillMode
	{
		WireFrame = 2,
		Opaque = 3,
	};

	enum class CullMode
	{
		Default = 0,
		Back,
		Front
	};

	enum class BlendMode
	{
		Default=0,
		OneMinus,
		Add,
		Multiply
	};

	enum class TopologyMode
	{
		Point = 0,
		Line,
		Triangle,
		TriangleFan,
		SurfacePatch
	};

	enum class Resource 
	{
		StructBuffer=0,		// e.g A structured buffer
		ConstBuffer,
		Texture,			// e.g A 2D texture
		Constants,			// e.g A constant buffer of 32-bit values
		TableConstBuffer,
		TableResourceBuffer,
		TableStructBuffer
	};

	struct PipelineInputDesc
	{
		UINT8 Count		= 1;// some apis (DX12) allow you to initialise multiple inputs at once
		UINT8 Register	= 0;
		UINT8 Space		= 0;
		UINT8 Flags		= 0;
		Resource Type = Resource::ConstBuffer;
	};

	/**
	 * @brief The pipeline description struct describes how the pipeline should behave.
	 *		  It is possible to specify what stages of the pipeline you desire as well
	 *		  as the shader which should be executed upon binding the pipeline.
	 */
	struct PipelineDesc
	{
		PipelineDesc() = default;
		~PipelineDesc() = default;

		std::vector<PipelineInputDesc> InputDesc;

		FillMode		Fill		= FillMode::Opaque;
		CullMode		Cull		= CullMode::Back;
		BlendMode		Blend		= BlendMode::OneMinus;
		TopologyMode	Topology	= TopologyMode::Triangle;
		BufferLayout	Layout;
	};

	// @brief Base class for describing the rendering pipeline.
	class RenderPipeline 
	{
	public:
		RenderPipeline(const PipelineDesc& desc);
		virtual ~RenderPipeline() = default;

		static ScopePointer<RenderPipeline> Create(PipelineDesc& desc);

		// @brief Updates pipeline resources
		virtual void Bind() = 0;
		virtual void Destroy() = 0;

		virtual void SetVertexShader(const std::string_view& name,		Shader* shader)	 = 0;
		virtual void SetPixelShader(const std::string_view& name,		Shader* shader)	 = 0;
		virtual void SetHullShader(const std::string_view& name,		Shader* shader)	 = 0;
		virtual void SetDomainShader(const std::string_view& name,		Shader* shader)	 = 0;
		virtual void SetGeometryShader(const std::string_view& name,	Shader* shader)	 = 0;

	protected:
		PipelineDesc Desc;
	};
}
