#pragma once
#include "Buffer.h"
#include "Framework/Core/core.h"

namespace Engine
{
	class GraphicsContext;

	struct FrameResource
	{
		virtual ~FrameResource() = default;

		static RefPointer<FrameResource> Create(GraphicsContext* graphicsContex, UINT passCount, UINT objectCount);
	};
}
