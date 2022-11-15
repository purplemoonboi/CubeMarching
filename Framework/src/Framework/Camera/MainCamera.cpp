#include "Framework/cmpch.h"
#include "MainCamera.h"


namespace Engine
{


	MainCamera::MainCamera(float width, float height, float nearPlane, float farPlane, float fov)
			:
		    DistanceToTarget(5.0f),
			Phi(DirectX::XM_PIDIV4),
			Theta(DirectX::XM_PI * 1.5f),
			AspectRatio(width/height),
			NearPlane(nearPlane),
			FarPlane(farPlane),
			Fov(fov),
			ViewportWidthHeight(width, height)
	{
		/**
		 *The window resized, so update the aspect ratioand recompute the projection matrix.
		 */ 
		DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(fov, (width / height), nearPlane, farPlane);
		DirectX::XMStoreFloat4x4(&Proj, P);
	}

	void MainCamera::Update(const AppTimeManager& deltaTime)
	{
		/**
		 *	Convert Spherical to Cartesian coordinates.
		 */ 
		const float x = DistanceToTarget * sinf(Phi) * cosf(Theta);
		const float z = DistanceToTarget * sinf(Phi) * sinf(Theta);
		const float y = DistanceToTarget * cosf(Phi);


		/**
		 * Build the view matrix.
		 */
		Position	= DirectX::XMVectorSet(x, y, z, 1.0f);
		Target		= DirectX::XMVectorZero();
		Up			= DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);


		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(Position, Target, Up);
		DirectX::XMStoreFloat4x4(&View, view);

		DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&World);
		DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&Proj);
		WorldViewProj = world * view * proj;
	}

	void MainCamera::SetPosition(float x, float y, float z)
	{
		DirectX::XMFLOAT3 currentPosition;
		DirectX::XMStoreFloat3(&currentPosition, Position);
		currentPosition.x += x;
		currentPosition.y += y;
		currentPosition.z += z;
		Position = DirectX::XMVectorSet(currentPosition.x, currentPosition.y, currentPosition.z, 1.0f);
	}

	void MainCamera::RecalculateAspectRatio(float width, float height, float nearPlane, float farPlane, float fov)
	{
		DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(fov, (width / height), nearPlane, farPlane);
		DirectX::XMStoreFloat4x4(&Proj, P);
	}
}

