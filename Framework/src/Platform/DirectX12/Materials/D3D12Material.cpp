#include "D3D12Material.h"

namespace Engine
{
	D3D12Material::D3D12Material(std::string&& name)
		:
		Name(name)
	{
	}

	void D3D12Material::SetAlbedo(float r, float g, float b, float a)
	{
		DiffuseAlbedo = { r,g,b,a };
	}

	void D3D12Material::SetRoughness(float roughness)
	{
		Roughness = roughness;
	}

	void D3D12Material::SetFresnel(float r, float g, float b)
	{
		FresnelR0 = { r,g,b };
	}

	void D3D12Material::SetBufferIndex(INT32 index)
	{
		MaterialBufferIndex = index;
	}
}
