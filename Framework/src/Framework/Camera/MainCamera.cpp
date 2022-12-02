#include "Framework/cmpch.h"
#include "MainCamera.h"

#include "Framework/Core/Log/Log.h"
#include "Framework/Core/Time/DeltaTime.h"


namespace Engine
{


	MainCamera::MainCamera(float width, float height, float nearPlane, float farPlane, float fov)
			:
		    DistanceToTarget(15),
			Phi(DirectX::XM_PIDIV4),
			Theta(DirectX::XM_PI * 1.5f),
			AspectRatio(width/height),
			NearPlane(nearPlane),
			FarPlane(farPlane),
			Fov(fov),
			ViewportWidthHeight(width, height),
			MinDistance(5.0f),
			MaxDistance(250.0f)
	{
		/**
		 *The window resized, so update the aspect ratioand recompute the projection matrix.
		 */ 
		DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(fov, (width / height), nearPlane, farPlane);
		DirectX::XMStoreFloat4x4(&Proj, P);
	}

	void MainCamera::Update(const float deltaTime)
	{
		/**
		 *	Convert Spherical to Cartesian coordinates.
		 */ 
		Position.x = DistanceToTarget * sinf(Phi) * cosf(Theta);
		Position.z = DistanceToTarget * sinf(Phi) * sinf(Theta);
		Position.y = DistanceToTarget * cosf(Phi);


		/**
		 * Build the view matrix.
		 */
		DirectX::XMVECTOR eye	= DirectX::XMVectorSet(Position.x, Position.y, Position.z, 1.0f);
		Target		= DirectX::XMVectorZero();
		Up			= DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);


		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eye, Target, Up);
		DirectX::XMStoreFloat4x4(&View, view);

	//	DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&World);
		DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&Proj);
	//	WorldViewProj = world * view * proj;


	}

	void MainCamera::UpdateCameraZenith(float pitch, float deltaTime)
	{
		Phi += pitch * deltaTime;
		MathHelper::Clamp(Phi, 0.1f, MathHelper::Pi - 0.1f);
	}

	void MainCamera::UpdateCamerasAzimuth(float yaw, float deltaTime)
	{
		Theta += yaw * deltaTime;

	}

	void MainCamera::UpdateCamerasDistanceToTarget(float delta, float deltaTime)
	{
		DistanceToTarget += delta * deltaTime;
		DistanceToTarget = MathHelper::Clamp(DistanceToTarget, MinDistance, MaxDistance);
	}

	void MainCamera::PassNextPosition(float x, float y, float z)
	{
		NextPosition.x = x;
		NextPosition.y = y;
		NextPosition.z = z;
	}

	void MainCamera::RecalculateAspectRatio(float width, float height, float nearPlane, float farPlane, float fov)
	{
		DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(fov, (width / height), nearPlane, farPlane);
		DirectX::XMStoreFloat4x4(&Proj, P);
	}
}

