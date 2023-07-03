#pragma once
#include <wrl/client.h>
#include "Framework/Core/core.h"
#include "Framework/Renderer/Buffers/Buffer.h"

#include <Platform/DirectX12/DirectX12.h>

#include "Framework/Renderer/Resources/Shader.h"

namespace Foundation::Graphics
{
	class Texture;

	class ComputeApi;
	class BufferLayout;
	class Shader;
	class GraphicsContext;

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
		StructBuffer=0,// e.g A structured buffer
		ConstBuffer,
		Texture,// e.g A 2D texture
		Constants,// e.g A constant buffer of 32-bit values
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


		PipelineInputDesc Input;

		FillMode		Fill		= FillMode::Opaque;
		CullMode		Cull		= CullMode::Back;
		BlendMode		Blend		= BlendMode::OneMinus;
		TopologyMode	Topology	= TopologyMode::Triangle;

	
		std::array<Shader*, 5> Shaders{nullptr};
	};

	// @brief Base class for describing the rendering pipeline.
	class RenderPipeline 
	{
	public:
		RenderPipeline() = default;
		virtual ~RenderPipeline() = default;

		static ScopePointer<RenderPipeline> Create(PipelineDesc& desc);

		// @brief Updates pipeline resources
		virtual void Invalidate() = 0;
		virtual void Bind() = 0;
		virtual void Destroy() = 0;

		virtual void SetVertexShader(const std::string_view& name,		Shader* shader)	 = 0;
		virtual void SetPixelShader(const std::string_view& name,		Shader* shader)	 = 0;
		virtual void SetHullShader(const std::string_view& name,		Shader* shader)	 = 0;
		virtual void SetDomainShader(const std::string_view& name,		Shader* shader)	 = 0;
		virtual void SetGeometryShader(const std::string_view& name,	Shader* shader)	 = 0;
		virtual void SetTaskShader(const std::string_view& name,		Shader* shader)	 = 0;
		virtual void SetMeshShader(const std::string_view& name,		Shader* shader)	 = 0;
		virtual void SetRaytracingShader(const std::string_view& name,	Shader* shader)	 = 0;

		virtual void SetComputeShader(const std::string_view& name,		Shader* shader)	 = 0;

		virtual void SetInput(const std::string_view& name, float value) = 0;
		virtual void SetInput(const std::string_view& name, INT32 value) = 0;
		virtual void SetInput(const std::string_view& name, bool value) = 0;

		virtual void SetInput(const std::string_view& name, VertexBuffer& buffer) = 0;
		virtual void SetInput(const std::string_view& name, IndexBuffer& buffer) = 0;

		[[nodiscard]] virtual ScopePointer<Texture> GetSceneTexture() const = 0;
		[[nodiscard]] virtual ScopePointer<Texture> GetSceneDepthTexture() const = 0;

	};
}
