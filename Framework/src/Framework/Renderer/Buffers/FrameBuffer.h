#pragma once
#include "Framework/Core/core.h"

namespace Foundation::Graphics
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
		INT32 OffsetX;
		INT32 OffsetY;
	};

	class FrameBuffer
	{
	public:
		FrameBuffer(const FrameBufferSpecifications& specs, ResourceFormat format)
			:
				FrameSpecs(specs)
		{}

		virtual ~FrameBuffer() = default;

		[[nodiscard]] const FrameBufferSpecifications& GetSpecifications() const { return FrameSpecs; }
		[[nodiscard]] INT32 GetWidth() const { return FrameSpecs.Height; };
		[[nodiscard]] INT32 GetHeight() const { return FrameSpecs.Width; };

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		virtual void SetBufferSpecifications(FrameBufferSpecifications& fbSpecs);
		virtual void OnResizeFrameBuffer(FrameBufferSpecifications& fbSpecs) = 0;
		virtual UINT64 GetFrameBuffer() const = 0;

		static ScopePointer<FrameBuffer> Create(const FrameBufferSpecifications& fBufferSpecs, ResourceFormat format);

	protected:
		FrameBufferSpecifications FrameSpecs;
		ResourceFormat Format;
	};

}

