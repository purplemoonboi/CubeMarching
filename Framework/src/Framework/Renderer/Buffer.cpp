#include "Framework/cmpch.h"
#include "Framework/Core/Log/Log.h"

#include "Buffer.h"
#include "Renderer.h"
#include "RendererAPI.h"

#include "Platform/DirectX12/DX12Buffer.h"

namespace Engine
{
	RefPointer<VertexBuffer> VertexBuffer::Create(GraphicsContext* const graphicsContext, UINT size, UINT vertexCount)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateRef<DX12VertexBuffer>(graphicsContext, size, vertexCount);
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	RefPointer<VertexBuffer> VertexBuffer::Create(GraphicsContext* const graphicsContext, const void* vertices, UINT size, UINT vertexCount)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:		CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::Api::OpenGL:		CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:		CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:		CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateRef<DX12VertexBuffer>(graphicsContext, vertices, size, vertexCount);
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	RefPointer<IndexBuffer> IndexBuffer::Create(GraphicsContext* const graphicsContext, UINT16* indices, UINT size, UINT indexCount)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:		CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::Api::OpenGL:		CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:		CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:		CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateRef<DX12IndexBuffer>(graphicsContext, indices, size, indexCount);
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	RefPointer<UploadBufferManager> UploadBufferManager::Create
	(
		GraphicsContext* const graphicsContext, 
		ScopePointer<FrameResource>* const frameResources,
		UINT count, 
		bool isConstant, 
		UINT frameResourceCount, 
		UINT renderItemsCount
	)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:		CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::Api::OpenGL:		CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan:		CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:		CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateRef<DX12UploadBufferManager>(graphicsContext, frameResources, count, isConstant, frameResourceCount, renderItemsCount);
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}
}
