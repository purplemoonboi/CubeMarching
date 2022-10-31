#include "Framework/cmpch.h"
#include "Framework/Core/Log/Log.h"

#include "Buffer.h"
#include "Renderer.h"
#include "RendererAPI.h"

#include "Platform/DirectX12/DX12Buffer.h"

namespace Engine
{
	RefPointer<VertexBuffer> VertexBuffer::Create(INT32 size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateRef<DX12VertexBuffer>(size);
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	RefPointer<VertexBuffer> VertexBuffer::Create(float* vertices, INT32 size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:		CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::Api::OpenGL:		CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:		CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:		CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateRef<DX12VertexBuffer>(vertices, size);
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	RefPointer<IndexBuffer> IndexBuffer::Create(INT32* indices, INT32 size)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:		CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::Api::OpenGL:		CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:		CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:		CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateRef<DX12IndexBuffer>(indices, size);
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}
}