#include "Framework/cmpch.h"
#include "PipelineStateObject.h"
#include "Framework/Renderer/Engine/Renderer.h"
#include "Platform/DirectX12/Copy/D3D12CopyContext.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"

namespace Foundation
{

	ScopePointer<PipelineStateObject> PipelineStateObject::Create
	(
		GraphicsContext* graphicsContext,
		Shader* vertexShader, 
		Shader* pixelShader, 
		FillMode fillMode 
	)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
		case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
		case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateScope<D3D12PipelineStateObject>(dynamic_cast<D3D12Context*>(graphicsContext), dynamic_cast<D3D12Shader*>(vertexShader), dynamic_cast<D3D12Shader*>(pixelShader), fillMode);
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
		}
	}

	ScopePointer<PipelineStateObject> PipelineStateObject::Create(
		GraphicsContext* graphicsContext,
		const PipelineResourceDesc& args,
		const PipelineDesc& desc
	)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
		case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
		case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateScope<D3D12PipelineStateObject>(dynamic_cast<D3D12Context*>(graphicsContext),args, desc);
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
		}
	}

	ScopePointer<PipelineStateObject> PipelineStateObject::Create(ComputeApi* computeContext, Shader* computeShader, ComPtr<ID3D12RootSignature> rootSignature)
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
