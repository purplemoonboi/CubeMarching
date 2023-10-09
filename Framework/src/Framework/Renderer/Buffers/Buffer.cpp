#include "Framework/cmpch.h"
#include "Framework/Core/Log/Log.h"

#include "Buffer.h"
#include "Framework/Renderer/Api/RendererAPI.h"

#include "Platform/DirectX12/Buffers/D3D12Buffers.h"

namespace Foundation::Graphics
{
	using namespace D3D12;

	ScopePointer<VertexBuffer> VertexBuffer::Create(UINT size, UINT vertexCount)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateScope<D3D12VertexBuffer>(size, vertexCount);
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}

	ScopePointer<VertexBuffer> VertexBuffer::Create(const void* vertices, UINT size, UINT vertexCount, bool isDynamic)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:		CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::Api::Vulkan:		CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateScope<D3D12VertexBuffer>(vertices, size, vertexCount, isDynamic);
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}



	ScopePointer<IndexBuffer> IndexBuffer::Create(UINT16* indices, UINT64 size, UINT indexCount)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:		CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::Api::Vulkan:		CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateScope<D3D12IndexBuffer>(indices, size, indexCount);
		}

		CORE_ASSERT(false, "Unknown renderer RendererAPI!");
		return nullptr;
	}


	DynamicBuffer::DynamicBuffer(const std::string_view& registeredName)
		:
		GPUResource(registeredName)
	{
	}

}
