#include "DX12Material.h"

namespace Engine
{
	DX12Material::DX12Material(std::string&& name)
		:
		Name(name)
	{
	}

	void DX12Material::SetAlbedo(float r, float g, float b, float a)
	{
		DiffuseAlbedo = { r,g,b,a };
	}

	void DX12Material::SetRoughness(float roughness)
	{
		Roughness = roughness;
	}

	void DX12Material::SetFresnel(float r, float g, float b)
	{
		FresnelR0 = { r,g,b };
	}

	void DX12Material::SetBufferIndex(INT32 index)
	{
		MaterialBufferIndex = index;
	}
}
