#pragma once
#include <intsafe.h>
#include <string>


namespace Engine
{
	class GraphicsContext;

	enum class TextureFormat
	{
		R8G8B8A8 = 0,
		R16G16B16A16,
		R32G32B32A32
	};

	enum class TextureDimension
	{
		One = 0,
		Two,
		Three
	};

	class Texture
	{
	public:
		Texture();
		virtual ~Texture();

		virtual void Create
		(
			UINT32 width,
			UINT32 height,
			GraphicsContext* context = nullptr,
			TextureDimension dimension = TextureDimension::Two,
			TextureFormat format = TextureFormat::R32G32B32A32
		) = 0;
	};
}
