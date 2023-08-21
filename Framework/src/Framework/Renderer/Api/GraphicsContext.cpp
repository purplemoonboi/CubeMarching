#include "Framework/cmpch.h"
#include "GraphicsContext.h"
#include "Framework/Core/Log/Log.h"
#include "Framework/Renderer/Renderer3D/Renderer.h"


#include "Platform/DirectX12/D3D12/D3D12.h"
#include "Platform/Windows/WindowsWindow.h"

namespace Foundation::Graphics
{
	using namespace D3D12;

	ScopePointer<GraphicsContext> GraphicsContext::Create(Window* window)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
			case RendererAPI::Api::DX12:
			{
				gD3D12Context.reset(new D3D12Context(dynamic_cast<WindowsWindow*>(window)));
			}
			case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
			default:
			CORE_ASSERT(false, "Unknown error!");
			return nullptr;
			
		}
	}
}