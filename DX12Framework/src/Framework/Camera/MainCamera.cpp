#include "Framework/cmpch.h"
#include "MainCamera.h"


namespace DX12Framework
{


	MainCamera::MainCamera(float width, float height, float nearPlane, float farPlane, float fov)
		:
		    DistanceToTarget(5.0f),
			Phi(DirectX::XM_PIDIV4),
			Theta(DirectX::XM_PI * 1.5f),
			AspectRatio(width/height),
			NearPlane(nearPlane),
			FarPlane(farPlane),
			Fov(fov)
	{
		float as = (width / height);
		// The window resized, so update the aspect ratio and recompute the projection matrix.
		DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(fov,as, nearPlane, farPlane);
		DirectX::XMStoreFloat4x4(&Proj, P);
	}

	void MainCamera::Update(const float deltaTime)
	{
		// Convert Spherical to Cartesian coordinates.
		float x = DistanceToTarget * sinf(Phi) * cosf(Theta);
		float z = DistanceToTarget * sinf(Phi) * sinf(Theta);
		float y = DistanceToTarget * cosf(Phi);


		// Build the view matrix.
		Position	= DirectX::XMVectorSet(x, y, z, 1.0f);
		Target		= DirectX::XMVectorZero();
		Up			= DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);


		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(Position, Target, Up);
		DirectX::XMStoreFloat4x4(&View, view);

		DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&World);
		DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&Proj);
		WorldViewProj = world * view * proj;


	}

	void MainCamera::RecalculateAspectRatio(float width, float height, float nearPlane, float farPlane, float fov)
	{
		DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(fov, (width / height), nearPlane, farPlane);
		DirectX::XMStoreFloat4x4(&Proj, P);
	}
}

