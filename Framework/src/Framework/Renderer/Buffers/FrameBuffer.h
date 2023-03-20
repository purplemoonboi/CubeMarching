#pragma once
#include "Framework/Core/core.h"

namespace Engine
{
	class GraphicsContext;

	struct  FrameBufferSpecifications
	{
		FrameBufferSpecifications()
			:
			Width(-1),
			Height(-1),
			Samples(1),
			SwapChainTarget(false)
		{}

		INT32 Width, Height;
		INT32 Samples;
		bool SwapChainTarget;

	};

	// @brief Abstract class, needs to be implemented on a per Api basis.
	class FrameBuffer
	{
	public:
		virtual ~FrameBuffer() = default;
		virtual const FrameBufferSpecifications& GetSpecifications() const = 0;
		virtual void Init(GraphicsContext* context) = 0;
		virtual void Bind() = 0;
		virtual void UnBind() = 0;
		virtual void RebuildFrameBuffer(INT32 width, INT32 height) = 0;
		virtual UINT64 GetFrameBuffer() const = 0;
		virtual INT32 GetWidth() const= 0;
		virtual INT32 GetHeight() const = 0;
		static ScopePointer<FrameBuffer> Create(const FrameBufferSpecifications& fBufferSpecs);
	};

}

