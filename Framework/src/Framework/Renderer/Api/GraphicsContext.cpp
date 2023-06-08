#include "Framework/cmpch.h"
#include "GraphicsContext.h"
#include "Framework/Core/Log/Log.h"
#include "Framework/Renderer/Renderer3D/Renderer.h"


#include "Platform/DirectX12/Api/D3D12Context.h"


namespace Foundation::Graphics
{
	using namespace D3D12;

	ScopePointer<GraphicsContext> GraphicsContext::Create(HWND hwnd, INT32 swapChainBufferWidth, INT32 swapChainBufferHeight)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
			case RendererAPI::Api::DX12:   return CreateScope<D3D12Context>(hwnd, swapChainBufferWidth, swapChainBufferHeight);
			case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
			default:
			CORE_ASSERT(false, "Unknown error!");
			return nullptr;
			
		}
	}
}