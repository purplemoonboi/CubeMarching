#pragma once
#include "Platform/DirectX12/DirectX12.h"

//#define GLM_ENABLE_EXPERIMENTAL
//#include "glm/gtx/quaternion.hpp"

#include <string>
#include "Framework/Core/Core.h"
#include "Framework/Renderer/Buffers/Buffer.h"
#include "Framework/Camera/MainCamera.h"
#include "Framework/Renderer/Engine/Mesh.h"
#include "Framework/Renderer/Textures/Texture.h"

namespace Foundation
{

	struct TagComponent
	{
		std::string tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& string)
			:
			tag(string) {}
	};

	struct TransformComponent
	{
		XMFLOAT3 Translation = { 0.f,0.f,0.f };
		XMFLOAT3 Rotation	  = { 0.f, 0.f,0.f };
		XMFLOAT3 Scale		  = { 1.f, 1.f, 1.f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const XMFLOAT3& trans)
			:
			Translation(trans){}

		[[nodiscard]] XMMATRIX GetTransform() const
		{
			const XMVECTOR rotQ = XMQuaternionRotationAxis(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMConvertToRadians(0.0f));
			
			return XMMatrixMultiply(
						XMMatrixMultiply(
							XMMatrixTranslation(Translation.x, Translation.y, Translation.z),  /* translate */
								XMMatrixRotationQuaternion(rotQ)),							   /* rotate */
									XMMatrixScaling(Scale.x, Scale.y, Scale.z));;			   /* scale */
		}

	};

	struct SpriteRendererComponent
	{
		XMFLOAT4 Colour = {1.0f, 1.0f, 1.0f, 1.0f};
		//RefPointer<MaterialInstance> material_instance;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const XMFLOAT4& col)
			:
			Colour(col){}
	};

	struct CameraComponent
	{
		MainCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct MeshComponent
	{
		MeshComponent() = default;
		

		bool WireFrame = false;
		MeshGeometry* Mesh;
	};

	//Forward Declarations
	class DeltaTime;
	class ScriptableEntity;

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)();
		void (*DestoryScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestoryScript = [](NativeScriptComponent* nsc) { delete nsc->instance; nsc->instance = nullptr; };
		}
	};

}


