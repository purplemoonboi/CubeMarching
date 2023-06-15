#include "Framework/cmpch.h"
#include "GraphicsContext.h"
#include "Framework/Core/Log/Log.h"
#include "Framework/Renderer/Renderer3D/Renderer.h"


#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/Windows/Win32Window.h"

namespace Foundation::Graphics
{
	using namespace D3D12;

	ScopePointer<GraphicsContext> GraphicsContext::Create(Window* window)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
			case RendererAPI::Api::DX12:   return CreateScope<D3D12Context>(dynamic_cast<Win32Window*>(window));
			case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
			default:
			CORE_ASSERT(false, "Unknown error!");
			return nullptr;
			
		}
	}
}