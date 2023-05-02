#include "Framework/cmpch.h"
#include "PipelineStateObject.h"
#include "Framework/Renderer/Engine/Renderer.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"

namespace Engine
{
	ScopePointer<PipelineStateObject> PipelineStateObject::CreateSkyPso
	(
		GraphicsContext* graphicsContext,
		Shader* vertexShader,
		Shader* pixelShader
	)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
		case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
		case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateScope<D3D12PipelineStateObject>(graphicsContext, vertexShader, pixelShader);
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
		}

	}

	ScopePointer<PipelineStateObject> PipelineStateObject::CreateShadowPso
	(
		GraphicsContext* graphicsContext,
		Shader* vertexShader, Shader* pixelShader, 
		const BufferLayout& layout
	)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
		case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
		case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateScope<D3D12PipelineStateObject>(graphicsContext, vertexShader, pixelShader, layout);
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
		}
	}

	ScopePointer<PipelineStateObject> PipelineStateObject::CreatePso
	(
		GraphicsContext* graphicsContext,
		Shader* vertexShader, 
		Shader* pixelShader, 
		const BufferLayout& layout,
		FillMode fillMode 
	)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
		case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
		case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateScope<D3D12PipelineStateObject>(graphicsContext, vertexShader, pixelShader, layout, fillMode);
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
		}
	}

	ScopePointer<PipelineStateObject> PipelineStateObject::CreateComputePso(ComputeApi* computeContext, Shader* computeShader, ComPtr<ID3D12RootSignature> rootSignature)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
		case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
		case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateScope<D3D12PipelineStateObject>(computeContext, computeShader, rootSignature);
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
		}
	}
}
