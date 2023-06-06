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
		static ScopePointer<Material> Create(std::string&& name);
		virtual void SetDiffuse(float r, float g, float b, float a) = 0;
		virtual void SetFresnel(float r, float g, float b) = 0;
		virtual void SetRoughness(float roughness) = 0;

		virtual void SetDiffuseTexIndex(INT32 index) = 0;
		virtual void SetNormalTexIndex(INT32 index) = 0;
		virtual void SetRoughnessTexIndex(INT32 index) = 0;
		virtual void SetDisplacementTexIndex(INT32 index) = 0;
		virtual void SetMaterialBufferIndex(UINT32 index) = 0;
		[[nodiscard]] virtual const std::string& GetName() const = 0;
		[[nodiscard]] virtual UINT32 GetMaterialIndex() const = 0;
	};

	class MaterialLibrary
	{
	public:

		static void Add(ScopePointer<Material> shader);

		static void Add(const std::string& name, ScopePointer<Material> shader);

		static Material* Get(const std::string& name);

		static UINT32 Size();

		static bool Exists(const std::string& name);

	private:
		static std::unordered_map<std::string, ScopePointer<Material>> Materials;

	};

}
