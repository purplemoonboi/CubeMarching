#include "RenderItems.h"
#include "Framework/Renderer/Api/RendererAPI.h"
#include "Framework/Renderer/Renderer3D/Renderer.h"

#include "Platform/DirectX12/RenderItems/D3D12RenderItem.h"

using namespace DirectX;

namespace Foundation::Graphics
{
	using namespace D3D12;

	ScopePointer<RenderItem> RenderItem::Create
	(
		MeshGeometry* geometry,
		class Material* material,
		const std::string& drawArgs,
		UINT constantBufferIndex
	)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
		case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
		case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateScope<D3D12RenderItem>(geometry, material, drawArgs, constantBufferIndex);
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;

		default:
			CORE_ASSERT(false, "An unknown error has occurred!"); return nullptr;
		}

	}



}

