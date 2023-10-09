#pragma once
#include <intsafe.h>
#include <string>
#include <unordered_map>
#include "Framework/Core/core.h"


namespace Foundation::Graphics
{
	class MemoryManager;
	class GraphicsContext;

	enum class ResourceFormat
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
		RGBA_UINT_UNORM = 28
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

	struct TextureTypeSize
	{
		static std::unordered_map<ResourceFormat, UINT64> TypeSize;
	};

	class Texture
	{
	public:
		Texture() = default;
		virtual ~Texture() = default;

		static ScopePointer<Texture> Create
		(
			const void* initData,
			UINT32 width,
			UINT32 height,
			UINT16 depth = 1u,
			ResourceFormat format = ResourceFormat::RGBA_FLOAT_32 
		);


		static ScopePointer<Texture> Create
		(
			const std::wstring& fileName,
			const std::string_view& name
		);

		virtual void LoadFromFile
		(
			const std::wstring& fileName,
			const std::string_view& name
		) = 0;

		virtual void Destroy() = 0;

		[[nodiscard]] virtual UINT64			GetWidth()				const = 0;
		[[nodiscard]] virtual UINT32			GetHeight()				const = 0;
		[[nodiscard]] virtual UINT16			GetDepth()				const = 0;
		[[nodiscard]] virtual TextureDimension	GetTextureDimension()	const = 0;
		[[nodiscard]] virtual ResourceFormat		GetTextureFormat()		const = 0;
		[[nodiscard]] virtual UINT64			GetTexture()			const = 0;

		virtual void SetWidth(UINT32 width) = 0;
		virtual void SetHeight(UINT32 height) = 0;
		virtual void SetDepth(UINT16 depth) = 0;

	};

	class TextureLibrary
	{
	public:

		static void Add(const std::string& name, ScopePointer<Texture> texture);

		static void Remove(const std::string& name);

		static Texture* GetTexture(const std::string_view& name);

		static bool Exists(const std::string_view& name);


	private:

		static inline std::unordered_map<std::string, ScopePointer<Texture>> Textures;

	};

}
