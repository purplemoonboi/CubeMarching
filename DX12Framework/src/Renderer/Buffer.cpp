#include "cmpch.h"
#include "Core/Log/Log.h"

#include "Buffer.h"
#include "Renderer.h"
#include "RendererAPI.h"

namespace DX12Framework
{
	RefPointer<VertexBuffer> VertexBuffer::Create(INT32 size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:   CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::API::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::API::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::API::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::API::DX12:   CORE_ASSERT(false, "DirectX 12 is not a supported api!"); return nullptr;
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	RefPointer<VertexBuffer> VertexBuffer::Create(float* vertices, INT32 size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:	  CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::API::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::API::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::API::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::API::DX12:   CORE_ASSERT(false, "DirectX 12 is not a supported api!"); return nullptr;
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	RefPointer<IndexBuffer> IndexBuffer::Create(INT32* indices, INT32 size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:	  CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::API::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::API::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::API::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::API::DX12:   CORE_ASSERT(false, "DirectX 12 is not a supported api!"); return nullptr;
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}
}