#include "Framework/cmpch.h"
#include "RenderPipeline.h"

#include "Framework/Renderer/Renderer3D/Renderer.h"
#include "Platform/DirectX12/Pipeline/D3D12RenderPipeline.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"

namespace Foundation::Graphics
{
	using namespace D3D12;

	RenderPipeline::RenderPipeline(const PipelineDesc& desc)
		:
		Desc(desc)
	{}

	ScopePointer<RenderPipeline> RenderPipeline::Create(PipelineDesc& desc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return { nullptr };
		case RendererAPI::Api::DX12:   return CreateScope<D3D12RenderPipeline>(desc);
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return { nullptr };
		}
		return{ nullptr };
	}

}
