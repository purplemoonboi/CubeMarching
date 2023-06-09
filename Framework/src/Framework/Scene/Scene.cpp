#include "Framework/cmpch.h"
#include "Framework/Renderer/Renderer3D/Renderer3D.h"

#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include "ScriptableEntity.h"

namespace Foundation
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

	void Scene::OnUpdate(AppTimeManager* time)
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

					nsc.Instance->OnUpdate(time);
				});
		}


		// Update object properties
		{
			auto view = Registry.view<TransformComponent>();
			for (auto entity : view)
			{
				auto transform = view.get<TransformComponent>(entity);
				
				
				
			}

		}


		//Get the active camera.
		MainCamera* camera = nullptr;
		XMMATRIX camTransform;
		
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

	void Scene::OnRender(AppTimeManager* time, bool wireframe)
	{
		//TODO: Implement camera component properly.
		/**
			MainCamera* camera = nullptr;
			auto group = Registry.view<CameraComponent>();

			for(auto entity : group)
			{
				auto cameraComponent = group.get<CameraComponent>(entity);

				if(cameraComponent.Primary)
				{
					camera = &cameraComponent.Camera;
					
				}
			}
		*/
		Renderer3D::BeginScene(SceneCamera, time, wireframe);

		

		Renderer3D::EndScene();

	}
}
