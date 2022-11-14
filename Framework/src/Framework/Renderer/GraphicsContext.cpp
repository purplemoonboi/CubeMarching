#include "Framework/cmpch.h"
#include "GraphicsContext.h"
#include "Renderer.h"
#include "Framework/Core/Log/Log.h"


#include "Platform/DirectX12/DX12GraphicsContext.h"


namespace Engine
{
	ScopePointer<GraphicsContext> GraphicsContext::Create(HWND windowHandle, INT32 swapChainBufferWidth, INT32 swapChainBufferHeight)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
			case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
			case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
			case RendererAPI::Api::DX12:   return CreateScope<DX12GraphicsContext>(windowHandle, swapChainBufferWidth, swapChainBufferHeight);
			case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
			default:
			CORE_ASSERT(false, "Unknown error!");
			return nullptr;
			
		}
	}
}