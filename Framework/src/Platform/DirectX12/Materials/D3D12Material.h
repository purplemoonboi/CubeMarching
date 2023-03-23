#pragma once
#include "MathHelper.h"
#include "Framework/Renderer/Resources/Material.h"


namespace Engine
{

	class D3D12Material : public Material
	{
	public:

		D3D12Material(std::string&& name);

		UINT32 GetMaterialIndex() const override { return MaterialBufferIndex; }
		const std::string& GetName() const override { return Name; }

		void SetDiffuse(float r, float g, float b, float a) override { DiffuseAlbedo = { r,g,b,a }; };
		void SetFresnel(float r, float g, float b)			override { FresnelR0 = { r,g,b }; }
		void SetRoughness(float roughness)					override { Roughness = roughness; }
		void SetMaterialBufferIndex(UINT32 index)			override { MaterialBufferIndex = index; };

		INT32 DirtyFrameCount = NUMBER_OF_FRAME_RESOURCES;
		INT32 MaterialBufferIndex = -1;
		INT32 DiffuseSrvHeapIndex = -1;
		INT32 NormalSrvHeapIndex = -1;

		// Material constant buffer data used for shading.
		DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
		float Roughness = .25f;
		float Metalness = 0.01f;
		
		DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();

		UINT32 DiffuseMapIndex = 0;
		UINT32 NormalMapIndex = 0;
		UINT32 RoughMapIndex = 0;
		UINT32 AoMapIndex = 0;
		UINT32 HeightMapIndex = 0;
		UINT32 UseWire = 0;

		std::string Name;

	};
}
