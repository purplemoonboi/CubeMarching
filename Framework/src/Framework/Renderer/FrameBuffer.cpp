#include "Framework/cmpch.h"
#include "Framework/Core/Log/Log.h"
#include "FrameBuffer.h"
#include "Renderer.h"

#include "Platform/DirectX12/DX12FrameBuffer.h"

namespace Engine
{
	ScopePointer<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecifications& fBufferSpecs)
	{
		switch (Renderer::GetAPI())
		{
		    case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
			case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
			case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
			case RendererAPI::Api::DX12:   return CreateScope<DX12FrameBuffer>(fBufferSpecs);
			case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
		}
	}
}