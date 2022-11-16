#include "RendererAPI.h"

#include "RenderItems.h"
#include "Renderer.h"


#include "Platform/DirectX12/DX12RenderItem.h"

using namespace DirectX;

namespace Engine
{
	ScopePointer<RenderItem> RenderItem::Create
	(
		Engine::MeshGeometry* geometry,
		std::string&& drawArgs,
		UINT constantBufferIndex,
		Transform transform
	)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
		case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
		case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateScope<DX12RenderItem>(geometry, std::move(drawArgs), constantBufferIndex, transform);
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;

		default:
			CORE_ASSERT(false, "An unknown error has occurred!"); return nullptr;
		}

	}


	



}

