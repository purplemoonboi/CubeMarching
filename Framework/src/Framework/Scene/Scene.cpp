#include "Framework/cmpch.h"
#include "Framework/Renderer/Engine/Renderer3D.h"

#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include "ScriptableEntity.h"

namespace Engine
{
	Scene::Scene(const std::string& name)
	{
		SceneCamera = new MainCamera(1920, 1080);
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		TagComponent& tag = entity.AddComponent<TagComponent>(name);
		tag.tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		Registry.destroy(entity);
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		//static_assert(false);
	}

	template<> void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<> void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetAspectRatio(Width, Height);
	}

	template<> void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<> void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<> void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}

	void Scene::OnUpdate(const float deltaTime, const float elapsedTime)
	{

		/** process scripts and entities */
		SceneCamera->Update();


		//Update Scripts
		{
			Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
				{
					//TODO: Needs to be moved to scene::onplay()
					if (!nsc.Instance)
					{
						nsc.Instance = nsc.InstantiateScript();
						nsc.Instance->entity = Entity{ entity, this };
						nsc.Instance->OnCreate();
					}

					nsc.Instance->OnUpdate(deltaTime);
				});
		}

		//Get the active camera.
		MainCamera* camera = nullptr;
		XMMATRIX camTransform;
		{
			auto group = Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : group)
			{
				auto [transform, camComponent] = group.get<TransformComponent, CameraComponent>(entity);
				if (camComponent.Primary)
				{
					camera = &camComponent.Camera;
					camTransform = transform.GetTransform();
					break;
				}
			}
		}



		//Render Scene
		if (camera != nullptr)
		{
			
		}


	}

	void Scene::OnRender(const float deltaTime, const float elapsedTime, const WorldSettings& settings, bool wireframe) const
	{
		/** process drawing instructions */

		Renderer3D::BeginScene(*SceneCamera, settings, deltaTime, wireframe, elapsedTime);


		

		Renderer3D::EndScene();
	}
}
