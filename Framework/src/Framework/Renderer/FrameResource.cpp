#include "Framework/cmpch.h"
#include "FrameResource.h"
#include "RendererAPI.h"

#include "Framework/Core/Log/Log.h"

#include "Platform/DirectX12/DX12FrameResource.h"

namespace Engine
{
	RefPointer<FrameResource> FrameResource::Create(GraphicsContext* graphicsContext, UINT passCount, UINT objectCount)
	{
		switch(RendererAPI::GetAPI())
		{
			case RendererAPI::Api::None: 	CORE_ASSERT(false, "Not a recognised api!");	return nullptr;
			case RendererAPI::Api::OpenGL:	CORE_ASSERT(false, "OpenGL is not a supported api!");	return nullptr;
			case RendererAPI::Api::Vulkan:	CORE_ASSERT(false, "Vulkan is not a supported api!");	return nullptr;
			case RendererAPI::Api::DX11:	CORE_ASSERT(false, "DirectX 11 is not a supported api!");	return nullptr;
			case RendererAPI::Api::DX12:	return CreateRef<DX12FrameResource>(graphicsContext, passCount, objectCount);
			default:
				return nullptr;
		}
	}
}
