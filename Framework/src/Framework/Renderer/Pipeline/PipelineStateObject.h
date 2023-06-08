#pragma once
#include <wrl/client.h>
#include "Framework/Core/core.h"
#include "Framework/Renderer/Buffers/Buffer.h"

#include <Platform/DirectX12/DirectX12.h>

#include "Framework/Renderer/Resources/Shader.h"

namespace Foundation::Graphics
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
		PipelineDesc(const PipelineDesc& other)
		{
			*this = other;
		}

		auto operator=(const PipelineDesc& other) -> PipelineDesc&
		{

			Input		= other.Input;
			Fill		= other.Fill;
			Cull		= other.Cull;
			Blend		= other.Blend;
			Topology	= other.Topology;

			for (UINT32 i = 0; i < other.Shaders.size(); ++i)
			{
				Shaders[i] = other.Shaders[i];
			}

			return *this;
		}


		PipelineInputDesc Input;

		FillMode		Fill		= FillMode::Opaque;
		CullMode		Cull		= CullMode::Back;
		BlendMode		Blend		= BlendMode::OneMinus;
		TopologyMode	Topology	= TopologyMode::Triangle;

	
		std::array<Shader*, 5> Shaders{nullptr};
	};

	// @brief Base class for describing the rendering pipeline.
	class PipelineStateObject 
	{
	public:
		PipelineStateObject() = default;
		virtual ~PipelineStateObject() = default;

		static ScopePointer<PipelineStateObject> Create
		(
			PipelineDesc& desc
		);

		static ScopePointer<PipelineStateObject> Create
		(
			Shader* vertexShader,
			Shader* pixelShader,
			FillMode fillMode = FillMode::Opaque
		);

		/*static ScopePointer<PipelineStateObject> Create
		(
			Shader* computeShader,
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature
		);*/

	protected:
		PipelineDesc Desc;
	};
}
