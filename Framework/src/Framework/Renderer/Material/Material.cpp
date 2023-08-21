#include "Material.h"

namespace Foundation::Graphics
{

	///// Material ///////////////////////////////////////////////////////



	///// Material Library //////////////////////////////////////////////


	void MaterialLibrary::Add(const std::string_view& name, RefPointer<Material> shader)
	{
	}

	RefPointer<Material> MaterialLibrary::Get(const std::string_view& name)
	{
		return nullptr;
	}

	bool MaterialLibrary::Exists(const std::string_view& name)
	{
		return false;
	}
}