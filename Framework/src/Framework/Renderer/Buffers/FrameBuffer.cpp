#include "Framework/cmpch.h"
#include "Framework/Core/Log/Log.h"
#include "FrameBuffer.h"
#include "Framework/Renderer/Engine/Renderer.h"

#include "Platform/DirectX12/Buffers/D3D12FrameBuffer.h"

namespace Foundation
{
	ScopePointer<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecifications& fBufferSpecs)
	{
		switch (Renderer::GetAPI())
		{
		    case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
			case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!");	 return nullptr;
			case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
			case RendererAPI::Api::DX12:   return CreateScope<D3D12FrameBuffer>(fBufferSpecs);
			case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
		}
	}
}