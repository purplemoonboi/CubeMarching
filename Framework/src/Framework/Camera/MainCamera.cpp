#include "Framework/cmpch.h"
#include "MainCamera.h"

#include "Framework/Core/Log/Log.h"
#include "Framework/Core/Time/DeltaTime.h"


namespace Foundation::Graphics
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

	void MainCamera::Update()
	{
		if(CameraState == CameraState::Orbit)
		{
			
		}
		else if(CameraState == CameraState::Walk)
		{
			XMVECTOR R = XMLoadFloat3(&Right);
			XMVECTOR U = XMLoadFloat3(&Up);
			XMVECTOR L = XMLoadFloat3(&Look);
			XMVECTOR P = XMLoadFloat3(&Position);

			// Keep camera's axes orthogonal to each other and of unit length.
			L = XMVector3Normalize(L);
			U = XMVector3Normalize(XMVector3Cross(L, R));

			// U, L already ortho-normal, so no need to normalize cross product.
			R = XMVector3Cross(U, L);

			// Fill in the view matrix entries.
			float x = -XMVectorGetX(XMVector3Dot(P, R));
			float y = -XMVectorGetX(XMVector3Dot(P, U));
			float z = -XMVectorGetX(XMVector3Dot(P, L));

			XMStoreFloat3(&Right, R);
			XMStoreFloat3(&Up, U);
			XMStoreFloat3(&Look, L);

			View(0, 0) = Right.x;
			View(1, 0) = Right.y;
			View(2, 0) = Right.z;
			View(3, 0) = x;

			View(0, 1) = Up.x;
			View(1, 1) = Up.y;
			View(2, 1) = Up.z;
			View(3, 1) = y;

			View(0, 2) = Look.x;
			View(1, 2) = Look.y;
			View(2, 2) = Look.z;
			View(3, 2) = z;

			View(0, 3) = 0.0f;
			View(1, 3) = 0.0f;
			View(2, 3) = 0.0f;
			View(3, 3) = 1.0f;

			DirtyFlag = false;
		}
		

	}

	void MainCamera::Walk(float deltaTime)
	{
		// mPosition += d*mLook
		XMVECTOR s = XMVectorReplicate(Speed * deltaTime);
		XMVECTOR l = XMLoadFloat3(&Look);
		XMVECTOR p = XMLoadFloat3(&Position);
		XMStoreFloat3(&Position, XMVectorMultiplyAdd(s, l, p));

		DirtyFlag = true;
	}

	void MainCamera::Strafe(float deltaTime)
	{
		XMVECTOR s = XMVectorReplicate(Speed * deltaTime);
		XMVECTOR r = XMLoadFloat3(&Right);
		XMVECTOR p = XMLoadFloat3(&Position);
		XMStoreFloat3(&Position, XMVectorMultiplyAdd(s, r, p));

		DirtyFlag = true;
	}

	void MainCamera::Ascend(float deltaTime)
	{
		XMVECTOR s = XMVectorReplicate(Speed * deltaTime);
		XMVECTOR r = XMLoadFloat3(&Up);
		XMVECTOR p = XMLoadFloat3(&Position);
		XMStoreFloat3(&Position, XMVectorMultiplyAdd(s, r, p));

		DirtyFlag = true;
	}

	void MainCamera::Pitch(float y)
	{
		// Make each pixel correspond to a quarter of a degree.
		float ry = AngularSpeed * y - PreviousCoords.y;
		const float dy = XMConvertToRadians(ry);

		XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&Right), dy);

		XMStoreFloat3(&Up, XMVector3TransformNormal(XMLoadFloat3(&Up), R));
		XMStoreFloat3(&Look, XMVector3TransformNormal(XMLoadFloat3(&Look), R));

		PreviousCoords.y = y;

		DirtyFlag = true;
	}

	void MainCamera::RotateY(float x)
	{
		float rx = AngularSpeed * x - PreviousCoords.x;


		// Make each pixel correspond to a quarter of a degree.
		const float dx = XMConvertToRadians(rx);

		XMMATRIX R = XMMatrixRotationY(dx);

		XMStoreFloat3(&Right, XMVector3TransformNormal(XMLoadFloat3(&Right), R));
		XMStoreFloat3(&Up, XMVector3TransformNormal(XMLoadFloat3(&Up), R));
		XMStoreFloat3(&Look, XMVector3TransformNormal(XMLoadFloat3(&Look), R));

		PreviousCoords.x = x;

		DirtyFlag = true;
	}

	void MainCamera::LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp)
	{
		XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
		XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
		XMVECTOR U = XMVector3Cross(L, R);

		XMStoreFloat3(&Position, pos);
		XMStoreFloat3(&Look, L);
		XMStoreFloat3(&Right, R);
		XMStoreFloat3(&Up, U);

		DirtyFlag = true;
	}

	void MainCamera::LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up)
	{
		XMVECTOR P = XMLoadFloat3(&pos);
		XMVECTOR T = XMLoadFloat3(&target);
		XMVECTOR U = XMLoadFloat3(&up);

		LookAt(P, T, U);

		DirtyFlag = true;
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
		Fov = 0.25f * MathHelper::Pi;
		AspectRatio = (width/height);
		NearPlane = nearPlane;
		FarPlane = farPlane;

		mNearWindowHeight = 2.0f * NearPlane * tanf(0.5f * Fov);
		mFarWindowHeight = 2.0f * FarPlane * tanf(0.5f * Fov);

		XMMATRIX P = XMMatrixPerspectiveFovLH(Fov, AspectRatio, NearPlane, FarPlane);
		XMStoreFloat4x4(&Proj, P);
	}

	INT MainCamera::GetProjectionType() const
	{
		return (INT)ProjectionType::Perspective;
	}

	float MainCamera::GetPerspectiveFOV() const
	{
		return Fov;
	}

	float MainCamera::GetPerspectiveNearClip() const
	{
		return NearPlane;
	}

	float MainCamera::GetPerspectiveFarClip() const
	{
		return FarPlane;
	}

	float MainCamera::GetOrthographicSize() const
	{
		return 0.0f;
	}

	float MainCamera::GetOrthographicNearClip() const
	{
		return 0.0f;
	}

	float MainCamera::GetOrthographicFarClip() const
	{
		return 1.0f;
	}

	void MainCamera::SetProjectionType(ProjectionType type)
	{
		ProjType = type;
	}

	void MainCamera::SetPerspectiveFOV(float fov)
	{
		Fov = fov;
	}

	void MainCamera::SetPerspectiveNearClip(float nearClip)
	{
		NearPlane = nearClip;
	}

	void MainCamera::SetPerspectiveFarClip(float farClip)
	{
		FarPlane = farClip;
	}

	void MainCamera::SetOrthographicSize(float orthoSize)
	{
	}

	void MainCamera::SetOrthographicNearClip(float orthoNearClip)
	{
	}

	void MainCamera::SetOrthographicFarClip(float orthoFarClip)
	{
	}

	void MainCamera::SetCameraFlySpeed(float speed)
	{
		Speed = speed;
	}

	void MainCamera::SetCameraAngularSpeed(float speed)
	{
		AngularSpeed = speed;
	}

	void MainCamera::SetMouseCoords(float x, float y)
	{
		PreviousCoords = { x, y };
	}
}

