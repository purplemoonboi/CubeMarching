#pragma once
#include <wrl/client.h>
#include "Framework/Core/core.h"
#include "Framework/Renderer/Buffers/Buffer.h"

#include <Platform/DirectX12/DirectX12.h>

#include "Framework/Renderer/Resources/Shader.h"

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

	enum class ResourceType : INT8
	{
		Buffer,// e.g A structured buffer
		Texture,// e.g A 2D texture
		Constants// e.g A constant buffer of 32-bit values
	};

	struct PipelineInput
	{
		UINT8 Count=1;// some apis (DX12) allow you to initialise multiple inputs at once
		ResourceType Type;
	};

	/**
	 * @brief The pipeline input description describes what resources you wish to bind to the
	 * pipeline. 
	 */
	struct PipelineResourceDesc
	{
		UINT8 RootArgCount = 1;// e.g How many resources in total do you expect to bind.
		std::vector<PipelineInput> Resources;
	};


	/**
	 * @brief The pipeline description struct describes how the pipeline should behave.
	 *		  It is possible to specify what stages of the pipeline you desire as well
	 *		  as the shader which should be executed upon binding the pipeline.
	 */
	struct PipelineDesc
	{
		PipelineDesc() = default;

		void AddShader(Shader* shader, ShaderType type)
		{
			Shaders[static_cast<INT32>(type)] = shader;
		}

		[[nodiscard]] const std::array<Shader*, 5>& GetShaders() const { return Shaders; }

		FillMode Fill;

	private:
		std::array<Shader*, 5> Shaders;
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
			Shader* vertexShader,
			Shader* pixelShader,
			FillMode fillMode = FillMode::Opaque
		);

		static ScopePointer<PipelineStateObject> Create
		(
			const PipelineResourceDesc& args,
			const PipelineDesc& desc
		);


		static ScopePointer<PipelineStateObject> Create
		(
			ComputeApi* computeContext,
			Shader* computeShader,
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature
		);
	};
}
