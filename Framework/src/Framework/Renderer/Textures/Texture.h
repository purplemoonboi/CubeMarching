#pragma once
#include <intsafe.h>
#include <string>
#include <unordered_map>
#include "Framework/Core/core.h"
#include "Framework/Renderer/Formats/ResourceFormats.h"

namespace Foundation::Graphics
{
	class MemoryManager;
	class GraphicsContext;

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
	protected:
		Texture(
			UINT32 width,
			UINT32 height,
			UINT16 depth = 1u,
			ResourceFormat format = ResourceFormat::R32G32B32A32_FLOAT)
			:
			Width(width)
			, Height(height)
			, Depth(depth)
			, Format(format)
		{}

		Texture() = default;
		virtual ~Texture() = default;

	public:
		static ScopePointer<Texture> Create
		(
			const void* initData,
			UINT32 width,
			UINT32 height,
			UINT16 depth = 1u,
			ResourceFormat format = ResourceFormat::R32G32B32A32_FLOAT
		);

		static ScopePointer<Texture> Create
		(
			const std::wstring& fileName,
			const std::string_view& name
		);


		virtual void Destroy() {};

		[[nodiscard]] UINT64			GetWidth()				const { return Width; }
		[[nodiscard]] UINT32			GetHeight()				const { return Height; }
		[[nodiscard]] UINT16			GetDepth()				const { return Depth; }
		[[nodiscard]] TextureDimension	GetTextureDimension()	const { return Dimension; }
		[[nodiscard]] ResourceFormat	GetTextureFormat()		const { return Format; }
		[[nodiscard]] virtual UINT64	GetTexture()			const { return 0; }

		virtual void SetWidth(UINT32 width) { Width = width; };
		virtual void SetHeight(UINT32 height) { Height = height; };
		virtual void SetDepth(UINT16 depth) { Depth = depth; };

	protected:
		UINT64 Width;
		UINT32 Height;
		UINT16 Depth;
		UINT MipLevels = -1;
		std::string Name;
		std::wstring FileName;

		ResourceFormat Format;
		TextureDimension Dimension;
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
