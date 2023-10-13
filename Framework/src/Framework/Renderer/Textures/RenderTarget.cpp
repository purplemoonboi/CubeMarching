#include "RenderTarget.h"
#include "Framework/Core/Log/Log.h"
#include "Framework/Renderer/Api/RendererAPI.h"

#include "Platform/DirectX12/Textures/D3D12RenderTarget.h"

namespace Foundation::Graphics
{

	using namespace D3D12;

	ScopePointer<RenderTarget> RenderTarget::Create(
		const void* initData, 
		UINT32 width, UINT32 height, 
		ResourceFormat format
	)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::Api::None: 	CORE_ASSERT(false, "Not a recognised api!");	return nullptr;
		case RendererAPI::Api::Vulkan:	CORE_ASSERT(false, "Vulkan is not a supported api!");	return nullptr;
		case RendererAPI::Api::DX12:	return CreateScope<D3D12RenderTarget>(initData, width, height, format);
		default:
			return nullptr;
		}
	}

}
