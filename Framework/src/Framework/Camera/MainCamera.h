#pragma once
#include <DirectXMath.h>

#include "../FDLuna/MathHelper.h"

namespace Engine
{
	class MainCamera
	{
	public:
		MainCamera(float width, float height, float nearPlane = 1.0f, float farPlane = 1000.0f, float fov = 0.25f * MathHelper::Pi);
		virtual ~MainCamera() = default;

		virtual void Update(const float deltaTime);

		// @brief Returns the final matrix used in the constant buffer
		const DirectX::XMMATRIX& GetWorldViewProjMat() const { return WorldViewProj; }

		// @brief Returns the position of the camera
		const DirectX::XMVECTOR& GetPosition() const { return Position; }

		// @brief Recalculates the cameras aspect ratio
		void RecalculateAspectRatio(float width, float height, float nearPlane = 1.0f, float farPlane = 1000.0f, float fov = 0.25f * MathHelper::Pi);


	public:
		// Getters and Setters


		void SetAspectRatio(float width, float height) { AspectRatio = (width / height); }

		float GetAspectRatio() const { return AspectRatio; }

	private:

		float DistanceToTarget;
		float Phi;
		float Theta;

		float NearPlane;
		float FarPlane;
		float Fov;

		DirectX::XMVECTOR Position;
		DirectX::XMVECTOR Target;
		DirectX::XMVECTOR Up;

		DirectX::XMFLOAT4X4 View  = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 Proj  = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();


		DirectX::XMMATRIX WorldViewProj;

		float AspectRatio;
	};
}

