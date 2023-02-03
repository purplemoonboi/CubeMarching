#include "Framework/cmpch.h"
#include "VertexArray.h"
#include "Framework/Renderer/Api/RendererAPI.h"

#include "Platform/DirectX12/Buffers/DX12VertexArray.h"

namespace Engine
{
	RefPointer<VertexArray> VertexArray::Create()
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "None is not a renderer api!"); return nullptr;
		case RendererAPI::Api::OpenGL: CORE_ASSERT(false, "OpenGL is not a supported api!"); return nullptr;
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX11:   CORE_ASSERT(false, "DirectX 11 is not a supported api!"); return nullptr;
		case RendererAPI::Api::DX12:   return CreateRef<D3D12VertexArray>();
		}

		CORE_ASSERT(false, "Unknown renderer API!");
		return nullptr;
	}
}