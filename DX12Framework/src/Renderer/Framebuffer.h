#pragma once
#include "Core/core.h"

namespace DX12Framework
{

	struct  FramebufferSpecifications
	{
		INT32 Width, Height;
		INT32 Samples = 1;
		bool SwapChainTarget = false;
		//FramebufferFormat format = 0;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;
		virtual const FramebufferSpecifications& GetSpecifications() const = 0;
		virtual void Bind() = 0;
		virtual void UnBind() = 0;
		virtual void Resize(INT32 width, INT32 height) = 0;
		virtual INT32 GetColourAttachmentRendererID() const = 0;
		static RefPointer<Framebuffer> Create(const FramebufferSpecifications& fBufferSpecs);
	};

}

