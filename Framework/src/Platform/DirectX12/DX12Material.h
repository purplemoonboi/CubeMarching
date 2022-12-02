#pragma once
#include "MathHelper.h"
#include "Framework/Renderer/Resources/Material.h"

namespace Engine
{



	class DX12Material : public Material
	{
	public:

		DX12Material(std::string&& name);

		const std::string& GetName() const override { return Name; }

		void SetAlbedo(float r, float g, float b, float a = 1.0f) override;

		void SetRoughness(float roughness) override;

		void SetFresnel(float r, float g, float b) override;

		void SetBufferIndex(INT32 index) override;

		INT32 GetBufferIndex() const override { return MaterialBufferIndex; }

		INT32 DirtyFrameCount = NUMBER_OF_FRAME_RESOURCES;

		INT32 MaterialBufferIndex = -1;

		INT32 DiffuseSrvHeapIndex = -1;

		INT32 NormalSrvHeapIndex = -1;

		const DirectX::XMFLOAT4X4& GetMaterialTransform() const { return MatTransform; }

		DirectX::XMFLOAT4 GetDiffuse() const { return DiffuseAlbedo; }

		DirectX::XMFLOAT3 GetFresnelR0() const { return FresnelR0; }

		float GetRoughness() const { return Roughness; }



	private:

		// Material constant buffer data used for shading.
		DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
		float Roughness = .25f;
		DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();

		std::string Name;

	};
}