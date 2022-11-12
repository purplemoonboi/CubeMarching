#include "Framework/cmpch.h"
#include "PipelineStateObject.h"
#include "Renderer.h"
#include "Platform/DirectX12/DX12PipelineStateObject.h"

namespace Engine
{
	RefPointer<PipelineStateObject> PipelineStateObject::Create
	(
		GraphicsContext* graphicsContext,
		const std::string& vertexShader,
		const std::string& pixelShader,
		const BufferLayout& layout,
		UINT backBufferFormat,
		UINT depthStencilFormal
	)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
		case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
		case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateRef<DX12PipelineStateObject>(graphicsContext, vertexShader, pixelShader, layout, backBufferFormat, depthStencilFormal);
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
		}

	}

	RefPointer<PipelineStateObject> PipelineStateObject::Create
	(
		GraphicsContext* graphicsContext,
		Shader* vertexShader, 
		Shader* pixelShader, 
		const BufferLayout& layout, 
		UINT backBufferFormat, 
		UINT depthStencilFormal
	)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
		case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
		case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateRef<DX12PipelineStateObject>(graphicsContext, vertexShader, pixelShader, layout, backBufferFormat, depthStencilFormal);
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
		}
	}

}
