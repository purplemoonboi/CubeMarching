#pragma once
#include <intsafe.h>
#include <array>
#include <string>
#include <unordered_map>
#include "Framework/Core/core.h"


namespace Foundation::Graphics
{
	class Texture;

	struct IColourRGBA
	{
		IColourRGBA(INT8 r, INT8 g, INT8 b, INT8 a)
			: R(r), G(g), B(b), A(a)
		{}
		INT8 R = 0;
		INT8 G = 0;
		INT8 B = 0;
		INT8 A = 0;
	};

	struct IColourRGB
	{
		IColourRGB(INT8 r, INT8 g, INT8 b)
			: R(r), G(g), B(b)
		{}

		INT8 R = 0;
		INT8 G = 0;
		INT8 B = 0;
		INT8 A = 0;
	};

	struct ColourRGBA
	{
		ColourRGBA(float r, float g, float b, float a)
			: R(r), G(g), B(b), A(a)
		{}

		float R = 0.f;
		float G = 0.f;
		float B = 0.f;
		float A = 0.f;
	};

	struct ColourRGB
	{
		ColourRGB(float r, float g, float b)
			: R(r), G(g), B(b)
		{}

		float R = 0.f;
		float G = 0.f;
		float B = 0.f;
	};

	class Material
	{
	public:
		virtual void SetDiffuse(float r, float g, float b, float a);
		virtual void SetFresnel(float r, float g, float b);
		virtual void SetRoughness(float roughness);

		[[nodiscard]] virtual const std::string& GetName() const;
	};

	class MaterialLibrary
	{
	public:

		/// <summary>
		/// 
		/// </summary>
		/// <param name="name"></param>
		/// <param name="shader"></param>
		static void Add(const std::string_view& name, RefPointer<Material> shader);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="name"></param>
		/// <returns></returns>
		static RefPointer<Material> Get(const std::string_view& name);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="name"></param>
		/// <returns></returns>
		static bool Exists(const std::string_view& name);

	private:

		/// <summary>
		/// 
		/// </summary>
		static std::unordered_map<std::string, RefPointer<Material>> Materials;

	};

}
