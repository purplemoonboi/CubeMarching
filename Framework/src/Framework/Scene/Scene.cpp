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

	void Scene::OnRender(AppTimeManager* time)
	{

		for (auto& pipeline : RenderPipelines)
		{
		}

		Renderer3D::BeginScene(SceneCamera, time);
		
		

		Renderer3D::EndScene();

	}

	void Scene::InsertRenderPipeline(std::string&& name, ScopePointer<RenderPipeline> pipeline)
	{
		CORE_ASSERT(pipeline, "Invalid pipeline");

		if(pipeline != nullptr)
		{
			if(RenderPipelines.find(name) != RenderPipelines.end())
			{
				CORE_WARNING("Pipeline already exists in library.");
				APP_WARNING("Pipeline already exists in library.");
				return;
			}

			RenderPipelines.emplace(name, std::move(pipeline));

		}

	}

	void Scene::RemovePipeline(std::string&& name)
	{

		if (RenderPipelines.find(name) == RenderPipelines.end())
		{
			CORE_WARNING("Pipeline does not exist in library.");
			APP_WARNING("Pipeline does not exist in library.");
			return;
		}

		auto pipeline = RenderPipelines.at(name).get();

		pipeline->Destroy();

	}

	RenderPipeline* Scene::GetRenderPipeline(const std::string& name) const
	{

		if(RenderPipelines.find(name) == RenderPipelines.end())
		{
			CORE_WARNING("Could not find render pipeline {0}, returning nullptr.");
			APP_WARNING("Could not find render pipeline {0}, returning nullptr.");
			return { nullptr };
		}

		return RenderPipelines.at(name).get();

	}
}
