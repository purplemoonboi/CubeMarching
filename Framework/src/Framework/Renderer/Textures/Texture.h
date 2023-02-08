#pragma once
#include <intsafe.h>
#include <string>


namespace Engine
{
	class MemoryManager;
	class GraphicsContext;

	enum class TextureFormat
	{
		UNKNOWN = 0,
		R_FLOAT_32 = 41,
		RG_FLOAT_32 = 16,
		RGB_FLOAT_32 = 6,
		RGBA_FLOAT_32 = 2,
		R_FLOAT_16 = 54,
		RG_FLOAT_16 = 34,
		RGBA_FLOAT_16 = 10,
		R_SINT_8 = 64,
		RG_SINT_8 = 52,
		RGBA_SINT_8 = 32,
		R_UINT_8 = 62,
		RG_UINT_8 = 50,
		RGB_UINT_8 = 7,
		RGBA_UINT_8 = 30,
	};

	enum class TextureDimension
	{
		One = 0,
		Two,
		Three,
		Cube
	};

	class Texture
	{
	public:
		Texture();
		virtual ~Texture();

		virtual void InitialiseResource
		(
			const void* initData,
			TextureDimension dimension = TextureDimension::Two,
			GraphicsContext * context = nullptr,
			MemoryManager* memManager = nullptr
		) = 0;


		virtual void LoadFromFile
		(
			const std::wstring& fileName
		) = 0;

		virtual UINT64 GetWidth() = 0;
		virtual UINT32 GetHeight() = 0;
		virtual UINT16 GetDepth() = 0;
		virtual TextureDimension GetTextureDimension() = 0;
		virtual TextureFormat GetTextureFormat() = 0;

	};
}
