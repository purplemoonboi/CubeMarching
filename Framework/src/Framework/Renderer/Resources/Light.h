#pragma once
#include <DirectXMath.h>

#include "MathHelper.h"

namespace Engine
{

	enum class AttenuationType
	{
		Constant = 0,
		Linear,
		Quadratic
	};

#define MaxLights 16

	struct Light
	{
		DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
		float FalloffStart = 1.0f;                          // point/spot light only
		DirectX::XMFLOAT3 Direction = { 0.5f, -1.0f, 0.0f };// directional/spot light only
		float FalloffEnd = 10.0f;                           // point/spot light only
		DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
		float SpotPower = 64.0f;
		DirectX::XMFLOAT3 Radiance = { 0.01, 0.01, 0.01 };

	};

	struct MaterialConstants
	{
		DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };

		DirectX::XMFLOAT3 FresnelR0 = { 0.05f, 0.05f, 0.05f };
		float Roughness = 0.25f;

		float Metalness = 0.01f;
		float UseTexture = 0;
		float UsePBR = 0;
		float Pad = 0.f;

		// Used in texture mapping.
		DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
	};
}

