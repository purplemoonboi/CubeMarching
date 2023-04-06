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
		float Pad = 0.0f;
	};

	struct MaterialConstants
	{
		DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
		float Roughness = 0.25f;
		DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
		INT32 DiffuseMapIndex;
		INT32 NormalMapIndex;
		INT32 RoughMapIndex;
		INT32 AoMapIndex;
		INT32 HeighMapIndex;
		UINT32 Wire;
		UINT32 Pad0 = 0;
		UINT32 Pad1 = 0;
		
	};
}

