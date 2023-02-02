#pragma once
#include <intsafe.h>
#include <string>
#include <unordered_map>
#include "Framework/Core/core.h"


namespace Engine
{
	constexpr INT32 NUMBER_OF_FRAME_RESOURCES = 3;


	class Material
	{
	public:
		static ScopePointer<Material> Create(std::string&& name);

		virtual void SetAlbedo(float r, float g, float b, float a = 1.0f) = 0;

		virtual void SetRoughness(float roughness) = 0;

		virtual void SetFresnel(float r, float g, float b) = 0;

		virtual void SetBufferIndex(INT32 index) = 0;

		[[nodiscard]] virtual INT32 GetBufferIndex() const = 0;

		[[nodiscard]] virtual const std::string& GetName() const = 0;

	protected:

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
