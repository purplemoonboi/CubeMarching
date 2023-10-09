#include "Framework/cmpch.h"
#include "Framework/Core/Log/Log.h"
#include "FrameBuffer.h"
#include "Framework/Renderer/Renderer3D/Renderer.h"

#include "Platform/DirectX12/Buffers/D3D12FrameBuffer.h"

namespace Foundation::Graphics
{
	using namespace D3D12;

	ScopePointer<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecifications& fBufferSpecs, ResourceFormat format)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Api::None:   CORE_ASSERT(false, "Not a recognised api!");              return nullptr;
		case RendererAPI::Api::Vulkan: CORE_ASSERT(false, "Vulkan is not a supported api!");     return nullptr;
		case RendererAPI::Api::DX12:   return CreateScope<D3D12FrameBuffer>(fBufferSpecs, format);

		}
	}

	void FrameBuffer::SetBufferSpecifications(FrameBufferSpecifications& fbSpecs)
	{
		FrameSpecs.Width = fbSpecs.Width;
		FrameSpecs.Height = fbSpecs.Height;
		FrameSpecs.OffsetX = fbSpecs.OffsetX;
		FrameSpecs.OffsetY = fbSpecs.OffsetY;
	}

	
}