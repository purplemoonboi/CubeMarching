#include "Texture.h"
#include "Framework/Renderer/Api/RendererAPI.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"

namespace Engine
{
	ScopePointer<Texture> Texture::Create
	(
		const void* initData,
		UINT32 width,
		UINT32 height,
		UINT16 depth,
		TextureDimension dimension,
		TextureFormat format 
	)
	{
		switch(RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None: 	CORE_ASSERT(false, "Not a recognised api!");	return nullptr;
		case RendererAPI::Api::OpenGL:	CORE_ASSERT(false, "OpenGL is not a supported api!");	return nullptr;
		case RendererAPI::Api::Vulkan:	CORE_ASSERT(false, "Vulkan is not a supported api!");	return nullptr;
		case RendererAPI::Api::DX11:	CORE_ASSERT(false, "DirectX 11 is not a supported api!");	return nullptr;
		case RendererAPI::Api::DX12:	return CreateScope<D3D12Texture>(initData, width, height, depth, dimension, format);
		default:
			return nullptr;
		}
	}
}
