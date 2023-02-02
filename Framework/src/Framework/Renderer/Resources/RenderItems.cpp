#include "RenderItems.h"
#include "Framework/Renderer/Api/RendererAPI.h"
#include "Framework/Renderer/Engine/Renderer.h"

#include "Platform/DirectX12/RenderItems/D3D12RenderItem.h"

using namespace DirectX;

namespace Engine
{
	ScopePointer<RenderItem> RenderItem::Create
	(
		Engine::MeshGeometry* geometry,
		Engine::Material* material,
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
		case RendererAPI::Api::DX12:   return CreateScope<D3D12RenderItem>(geometry, material, std::move(drawArgs), constantBufferIndex, transform);
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;

		default:
			CORE_ASSERT(false, "An unknown error has occurred!"); return nullptr;
		}

	}


	



}

