#pragma once
#include <intsafe.h>
#include <string>

#include "Framework/Core/core.h"


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
		Unknown = 0,
		Buffer = 1,
		Tex1D = 2,
		Tex1DArray = 3,
		Tex2D = 4,
		Tex2DArray = 5,
		Tex2DMa = 6,
		Tex2DMArray = 7,
		Tex3D = 8,
		TexCube = 9,
		TexCubeArray = 10,
		RaytracingAccelerationStructure = 11
	};

	class Texture
	{
	public:
		Texture() = default;
		virtual ~Texture() = 0;

		static ScopePointer<Texture> Create
		(
			const void* initData,
			UINT32 width,
			UINT32 height,
			UINT16 depth = 1,
			TextureDimension dimension = TextureDimension::Tex2D,
			TextureFormat format = TextureFormat::RGBA_UINT_8
		);

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
