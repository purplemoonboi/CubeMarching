#pragma once
#include <DirectXMath.h>
#include "Framework/Core/Time/AppTimeManager.h"
#include "../FDLuna/MathHelper.h"

namespace Engine
{
	class MainCamera
	{
	public:
		MainCamera(float width, float height, float nearPlane = 1.0f, float farPlane = 1000.0f, float fov = 0.25f * MathHelper::Pi);
		virtual ~MainCamera() = default;


		virtual void Update(const AppTimeManager& deltaTime);

		// @brief Returns the final matrix used in the constant buffer
		const DirectX::XMMATRIX& GetWorldViewProjMat() const { return WorldViewProj; }


		void SetPosition(float x, float y, float z);
		// @brief Returns the position of the camera
		const DirectX::XMVECTOR& GetPosition() const { return Position; }

		// @brief - Recalculates the cameras aspect ratio
		void RecalculateAspectRatio(float width, float height, float nearPlane = 1.0f, float farPlane = 1000.0f, float fov = 0.25f * MathHelper::Pi);

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

		float Phi;
		float Theta;

		float NearPlane;
		float FarPlane;
		float Fov;

		float AspectRatio;

		DirectX::XMVECTOR Position;
		DirectX::XMVECTOR Target;
		DirectX::XMVECTOR Up;

		DirectX::XMMATRIX WorldViewProj;

		DirectX::XMFLOAT2 ViewportWidthHeight;

		DirectX::XMFLOAT4X4 View  = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 Proj  = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();

	};
}

