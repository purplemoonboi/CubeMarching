#pragma once
#include "Framework/Renderer/Textures/Texture.h"

namespace Foundation::Graphics
{
	class RenderTarget : public Texture
	{
	public:
		RenderTarget() = default;
		virtual ~RenderTarget() = default;

		static ScopePointer<RenderTarget> Create
		(
			const void* initData,
			UINT32 width,
			UINT32 height,
			TextureFormat format = TextureFormat::RGBA_UINT_UNORM
		);

		virtual void Bind(GraphicsContext* context) = 0;
		virtual void UnBind(GraphicsContext* context) = 0;
		virtual void OnResize(INT32 width, INT32 height) = 0;

	};
}
