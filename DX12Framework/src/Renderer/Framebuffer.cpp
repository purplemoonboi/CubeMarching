#include "cmpch.h"
#include "Core/Log/Log.h"
#include "Framebuffer.h"
#include "Renderer.h"

namespace DX12Framework
{
	RefPointer<Framebuffer> Framebuffer::Create(const FramebufferSpecifications& fBufferSpecs)
	{
		switch (Renderer::GetAPI())
		{
		    case RendererAPI::API::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
			case RendererAPI::API::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
			case RendererAPI::API::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
			case RendererAPI::API::DX12:   CORE_ASSERT(false, "DirectX 12 is not a supported api!"); return nullptr;
			case RendererAPI::API::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
		}
	}
}