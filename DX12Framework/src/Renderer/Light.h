#pragma once
#include <DirectXMath.h>

namespace DX12Framework
{

	enum class AttenuationType
	{
		Constant = 0,
		Linear,
		Quadratic
	};

	class Light
	{
	public:

		Light() = default;
		Light(const Light& other) = default;
		Light(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 direction, DirectX::XMFLOAT4 colour = { 1.0f, 1.0f, 1.0f, 1.0f });
		virtual ~Light() = default;

	public:

		const DirectX::XMFLOAT4& GetColour() const { return Albedo; }
		void SetColour(DirectX::XMFLOAT4 colour) { Albedo = colour; }

		const DirectX::XMFLOAT4& GetSpecularColour() const { return Specular; }
		void SetSpecularColour(DirectX::XMFLOAT4 specular) { Specular = specular; }

		const DirectX::XMFLOAT3& GetPosition() const { return Position; }
		void SetPosition(DirectX::XMFLOAT3 position) { Position = position; }

		const DirectX::XMFLOAT3& GetDirection() const { return Direction; }
		void SetDirection(DirectX::XMFLOAT3 direction) { Direction; }

		const DirectX::XMFLOAT3& GetEulerRotation() const { return EulerRotation; }
		void SetEulerRotation(DirectX::XMFLOAT3 rotation) { EulerRotation = rotation; }

		const DirectX::XMFLOAT3& GetAttenuation() const { return Attenuation; }
		void SetAttenuation(AttenuationType attenuationType);

		const float GetRange() const { return Range; }
		void SetRange(float range) { Range = range; }

		const float GetIntensity() const { return Intensity; }
		void SetIntensity(float intensity) { Intensity = intensity; }

		const float GetSpotPower() const { return SpotPower; }
		void SetSpotPower(float spotPower) { SpotPower = spotPower; }

		const float GetSpecularPower() const { return SpecularPower; }
		void SetSpecularPower(float specularPower) { SpecularPower = specularPower; }


	protected:

		DirectX::XMFLOAT4 Albedo;
		DirectX::XMFLOAT4 Specular;

		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Direction;
		DirectX::XMFLOAT4 Rotation;
		DirectX::XMFLOAT3 EulerRotation;
		DirectX::XMFLOAT3 Attenuation;
		
		float Range;
		float Intensity;
		float SpotPower;
		float SpecularPower;

	private:

	};
}

