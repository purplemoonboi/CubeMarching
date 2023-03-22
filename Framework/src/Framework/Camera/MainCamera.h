#pragma once
#include <DirectXMath.h>
#include "../FDLuna/MathHelper.h"

namespace Engine
{

	using namespace DirectX;

	

	class MainCamera
	{
	public:
		enum class ProjectionType
		{
			Orthographic = 0,
			Perspective,
		};

		enum class CameraState
		{
			Idle = 0,
			Orbit,
			Walk
		};

		MainCamera() = default;

		MainCamera(float width, float height, float nearPlane = 1.0f, float farPlane = 1000.0f, float fov = 0.25f * MathHelper::Pi);
		virtual ~MainCamera() = default;


		void Update();

		void Walk(float deltaTime);
		void Strafe(float deltaTime);
		void Ascend(float deltaTime);
		void Pitch(float x);
		void RotateY(float y);

		void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp);
		void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);

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

		const DirectX::XMFLOAT3& GetForward() const { return Look; };

		// @brief - Returns the camera's view matrix.
		const DirectX::XMFLOAT4X4& GetView()		const { return View; }

		// @brief - Returns the camera's projection matrix.
		const DirectX::XMFLOAT4X4& GetProjection()  const { return Proj; }

		// @brief - Returns the camera's world matrix.
		const DirectX::XMFLOAT4X4& GetWorld()		const { return World; }


		[[nodiscard]]INT GetProjectionType()const ;
		[[nodiscard]]float GetPerspectiveFOV()const ;
		[[nodiscard]]float GetPerspectiveNearClip()	const ;
		[[nodiscard]]float GetPerspectiveFarClip()const ;
		[[nodiscard]]float GetOrthographicSize()const ;
		[[nodiscard]]float GetOrthographicNearClip()const ;
		[[nodiscard]]float GetOrthographicFarClip()	const ;

		void SetProjectionType(ProjectionType type);
		void SetPerspectiveFOV(float fov);
		void SetPerspectiveNearClip(float nearClip)	;
		void SetPerspectiveFarClip(float farClip);
		void SetOrthographicSize(float orthoSize);
		void SetOrthographicNearClip(float orthoNearClip);
		void SetOrthographicFarClip(float orthoFarClip);

		void SetCameraFlySpeed(float speed);
		[[nodiscard]] float GetCameraFlySpeed() const { return Speed; }
		void SetCameraAngularSpeed(float speed);
		[[nodiscard]] float GetCameraAngularSpeed() const { return AngularSpeed; }

		void SetMouseCoords(float x, float y);

		// @brief - Update the camera's aspect ratio when the viewport has changed dimensions.
		// @param[in] - The width of the camera's viewport
		// @param[in] - The height of the camera's viewport
		void SetAspectRatio(float width, float height) { AspectRatio = (width / height); }

		// @brief - Returns the camera's current aspect ratio
		float GetAspectRatio() const { return AspectRatio; }

		// @brief - Returns the width and height of the camera's viewport
		const DirectX::XMFLOAT2 GetBufferDimensions() const { return ViewportWidthHeight; }

		[[nodiscard]] CameraState GetCameraState() const { return CameraState; }

	private:

		ProjectionType ProjType;
		CameraState CameraState = CameraState::Walk;

		bool DirtyFlag = true;
		float DistanceToTarget;
		float MinDistance;
		float MaxDistance;

		float Phi;
		float Theta;

		float NearPlane;
		float FarPlane;
		float Fov;

		XMFLOAT2 ViewportWidthHeight;
		XMFLOAT2 PreviousCoords = {0.0f, 0.0f};
		float mNearWindowHeight = 0.0f;
		float mFarWindowHeight = 0.0f;

		float AspectRatio;
		float Speed = 10.0f;
		float AngularSpeed = 1.0f;

		// Camera coordinate system with coordinates relative to world space.
		XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
		XMFLOAT3 Right = { 1.0f, 0.0f, 0.0f };
		XMFLOAT3 Up = { 0.0f, 1.0f, 0.0f };
		XMFLOAT3 Look = { 0.0f, 0.0f, 1.0f };

		XMMATRIX WorldViewProj;


		XMFLOAT4X4 View  = MathHelper::Identity4x4();
		XMFLOAT4X4 Proj  = MathHelper::Identity4x4();
		XMFLOAT4X4 World = MathHelper::Identity4x4();

	};
}

