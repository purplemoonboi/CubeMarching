#include "Framework/cmpch.h"
#include "MainCamera.h"

#include "Framework/Core/Log/Log.h"
#include "Framework/Core/Time/DeltaTime.h"


namespace Engine
{


	MainCamera::MainCamera(float width, float height, float nearPlane, float farPlane, float fov)
			:
		    DistanceToTarget(15),
			Phi(XM_PIDIV4),
			Theta(XM_PI * 1.5f),
			AspectRatio(width/height),
			NearPlane(nearPlane),
			FarPlane(farPlane),
			Fov(fov),
			ViewportWidthHeight(width, height),
			MinDistance(5.0f),
			MaxDistance(250.0f)
	{
		Position = { 0.f, 5.f, 10.f };
		/**
		 *The window resized, so update the aspect ratioand recompute the projection matrix.
		 */ 
		XMMATRIX P = XMMatrixPerspectiveFovLH(fov, (width / height), nearPlane, farPlane);
		XMStoreFloat4x4(&Proj, P);
	}

	void MainCamera::Update(const float deltaTime)
	{
		/**
		 *	Convert Spherical to Cartesian coordinates.
		 */ 
		float px = sinf(Phi) * cosf(Theta);
		float pz = sinf(Phi) * sinf(Theta);

		float py = cosf(Phi);


		/**
		 * Build the view matrix.
		 */
		XMVECTOR eye = XMLoadFloat3(&Position);
		//XMVECTOR eye = XMVectorSet(px, pz, py, 1);


		Target = XMVectorAdd(eye, XMVectorSet(  px, py, pz, 1));
		//Target = XMVectorSet(0, 0, 0, 1);
		Up			= XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);


		XMMATRIX view = XMMatrixLookAtLH(eye, Target, Up);
		XMStoreFloat4x4(&View, view);

		//Target = XMVector3Normalize(Target);
		XMStoreFloat3(&ForwardF3, Target);

	//	XMMATRIX world = XMLoadFloat4x4(&World);
		XMMATRIX proj = XMLoadFloat4x4(&Proj);
	//	WorldViewProj = world * view * proj;

	}

	void MainCamera::UpdateCameraZenith(float pitch, float deltaTime)
	{
		
		float rPitch = XMConvertToRadians(pitch);
		Phi += rPitch * deltaTime;
		Phi  = MathHelper::Clamp(Phi, XMConvertToRadians(20), XMConvertToRadians(160));

		CORE_TRACE("Zenith {0} Pitch {1}", XMConvertToDegrees(Phi), pitch);
	}

	void MainCamera::UpdateCamerasAzimuth(float yaw, float deltaTime)
	{
		float rYaw = XMConvertToRadians(yaw);
		Theta += rYaw * deltaTime;

		CORE_TRACE("Azimuth {0}, Yaw {1} ", XMConvertToDegrees(Theta), yaw);
	}

	void MainCamera::UpdateCamerasDistanceToTarget(float delta, float deltaTime)
	{
		DistanceToTarget += delta * deltaTime;
		DistanceToTarget = MathHelper::Clamp(DistanceToTarget, MinDistance, MaxDistance);
	}

	void MainCamera::SetPosition(XMFLOAT3 position)
	{
		Position.x = position.x;
		Position.y = position.y;
		Position.z = position.z;
	}

	void MainCamera::RecalculateAspectRatio(float width, float height, float nearPlane, float farPlane, float fov)
	{
		XMMATRIX P = XMMatrixPerspectiveFovLH(fov, (width / height), nearPlane, farPlane);
		XMStoreFloat4x4(&Proj, P);
	}
}

