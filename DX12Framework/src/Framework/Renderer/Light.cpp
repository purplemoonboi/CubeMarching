#include "Framework/cmpch.h"
#include "Light.h"

namespace DX12Framework
{
	Light::Light(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 direction, DirectX::XMFLOAT4 colour)
		:
		Position(position),
		Direction(direction),
		Albedo(colour),
		Specular(1.0f, 1.0f, 1.0f, 1.0f),
		Rotation(0.0f, 0.0f, 0.0f, 1.0f),
		EulerRotation(0.0f, 0.0f, 0.0f),
		Attenuation(0.0f, 1.0f, 0.0f),
		Range(10.0f),
		Intensity(1.0f),
		SpotPower(2.0f),
		SpecularPower(2.0f)
	{
	}

	void Light::SetAttenuation(AttenuationType attenuationType)
	{
		switch (attenuationType)
		{
		case AttenuationType::Constant:
		{
			Attenuation = { 1.0f, 0.0f, 0.0f };
			break;
		}
		case AttenuationType::Linear:
		{
			Attenuation = { 0.0f, 1.0f, 0.0f };
			break;
		}
		case AttenuationType::Quadratic:
		{
			Attenuation = { 0.0f, 0.0f, 1.0f };
			break;
		}
		}
	}
}