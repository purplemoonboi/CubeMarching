#pragma once

#include <DirectXCollision.h>
#include <DirectXMath.h>
#include <MathHelper.h>

namespace Engine
{
	struct WorldSettings
	{
		

		DirectX::XMFLOAT4 SunColour = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT3 SunDirection = { -0.3f, -0.9f, 0.0f };
		DirectX::XMFLOAT3 SunPosition = { 0, -10, -10.f };

		DirectX::XMFLOAT4X4 SunView = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 SunProjection = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 SunShadowTransform = MathHelper::Identity4x4();

		float SunRotationAngle = 45.0f;
		float SunNear = 0.0f;
		float SunFar = 0.0f;

		DirectX::BoundingSphere SceneBounds;
		WorldSettings()
		{
			SceneBounds.Center = { 0.0f, 0.0f, 0.0f };
			SceneBounds.Radius = sqrtf(10.0f * 10.0f + 15.0f * 15.0f);
		}
	};

}
