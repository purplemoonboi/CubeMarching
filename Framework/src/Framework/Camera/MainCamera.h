#pragma once
#include <DirectXMath.h>
#include "../FDLuna/MathHelper.h"

namespace Engine
{

	using namespace DirectX;

	class MainCamera
	{
	public:
		MainCamera(float width, float height, float nearPlane = 1.0f, float farPlane = 1000.0f, float fov = 0.25f * MathHelper::Pi);
		virtual ~MainCamera() = default;


		virtual void Update(const float deltaTime);

		// @brief Returns the final matrix used in the constant buffer
		const DirectX::XMMATRIX& GetWorldViewProjMat() const { return WorldViewProj; }

		// @brief - Update the camera's current zenith angle.
		void UpdateCameraZenith(float pitch, float deltaTime);

		// @brief - Update the camera's current azimuth angle.
		void UpdateCamerasAzimuth(float yaw, float deltaTime);

		// @brief - Update the camera's distance to the target object.
		void UpdateCamerasDistanceToTarget(float delta, float deltaTime);

		void SetPosition(DirectX::XMFLOAT3 position);
		// @brief Returns the position of the camera
		const DirectX::XMFLOAT3 GetPosition() const { return Position; }

		// @brief - Recalculates the cameras aspect ratio
		void RecalculateAspectRatio(float width, float height, float nearPlane = 1.0f, float farPlane = 1000.0f, float fov = 0.25f * MathHelper::Pi);

		const DirectX::XMFLOAT3& GetForward() const { return ForwardF3; };

		/**
		 *
		 *	Camera matrices
		 *
		 */

		// @brief - Returns the camera's view matrix.
		const DirectX::XMFLOAT4X4& GetView()		const { return View; }

		// @brief - Returns the camera's projection matrix.
		const DirectX::XMFLOAT4X4& GetProjection()  const { return Proj; }

		// @brief - Returns the camera's world matrix.
		const DirectX::XMFLOAT4X4& GetWorld()		const { return World; }

	public:

		// @brief - Update the camera's aspect ratio when the viewport has changed dimensions.
		// @param[in] - The width of the camera's viewport
		// @param[in] - The height of the camera's viewport
		void SetAspectRatio(float width, float height) { AspectRatio = (width / height); }

		// @brief - Returns the camera's current aspect ratio
		float GetAspectRatio() const { return AspectRatio; }

		// @brief - Returns the width and height of the camera's viewport
		const DirectX::XMFLOAT2 GetBufferDimensions() const { return ViewportWidthHeight; }

	private:

		float DistanceToTarget;
		float MinDistance;
		float MaxDistance;

		float Phi;
		float Theta;

		float NearPlane;
		float FarPlane;
		float Fov;

		float AspectRatio;

		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 NextPosition;


		DirectX::XMVECTOR Target;
		DirectX::XMVECTOR Up;

		DirectX::XMFLOAT3 ForwardF3;
		DirectX::XMFLOAT3 UpF3;

		DirectX::XMMATRIX WorldViewProj;

		DirectX::XMFLOAT2 ViewportWidthHeight;

		DirectX::XMFLOAT4X4 View  = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 Proj  = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();

	};
}

