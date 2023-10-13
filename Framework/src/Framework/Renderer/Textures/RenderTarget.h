#pragma once
#include "Framework/Renderer/Textures/Texture.h"

namespace Foundation::Graphics
{
	class RenderTarget
	{
	public:
		virtual ~RenderTarget() = default;

		static ScopePointer<RenderTarget> Create
		(
			const void* initData,
			UINT32 width,
			UINT32 height,
			ResourceFormat format = ResourceFormat::B8G8R8A8_UNORM
		);

		virtual void LoadFromFile(const std::wstring& fileName, const std::string& name) = 0;
		virtual void Bind() = 0;
		virtual void OnResize(UINT32 width, UINT32 height) = 0;
		virtual void OnDestroy() = 0;
		virtual void SetWidth(UINT32 width) = 0;
		virtual void SetHeight(UINT32 height) = 0;
		virtual void SetFormat(ResourceFormat format) = 0;

		[[nodiscard]] virtual UINT32 GetWidth() const = 0;
		[[nodiscard]] virtual UINT32 GetHeight() const = 0;
		[[nodiscard]] virtual ResourceFormat GetTextureFormat() const = 0;
		[[nodiscard]] virtual void* GetTexture() const = 0;
	};
}
