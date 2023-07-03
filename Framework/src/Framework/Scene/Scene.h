#pragma once
#include "entt.hpp"
#include "Framework/Core/Core.h"
#include "Framework/Core/Time/AppTimeManager.h"
#include "Framework/Renderer/Pipeline/RenderPipeline.h"

namespace Foundation
{
	namespace Editor
	{
		class SceneHierarchyPanel;
	}

	namespace Graphics 
	{
		class MainCamera;
	}
	using namespace Graphics;


	struct WorldSettings
	{
		float SunDirection[3] = { -0.3f, -0.9f, 0.0f };
		float SunColour[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		//INT32 ViewportX0 = 0;
		//INT32 ViewportY0 = 0;
		//INT32 ViewportX1 = 1920;
		//INT32 ViewportY1 = 1080;
	};


	class Entity;
	
	class Scene
	{
		friend class Entity;
		friend class SceneSerializer;
		friend class Editor::SceneHierarchyPanel;
	public:
		Scene() = delete;
		DISABLE_COPY_AND_MOVE(Scene);
		explicit Scene(const std::string& name);

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnUpdate(AppTimeManager* time);
		void OnRender(AppTimeManager* time);

	public:
		
		void InsertRenderPipeline(std::string&& name, ScopePointer<RenderPipeline> pipeline);

		void RemovePipeline(std::string&& name);

		[[nodiscard("")]] RenderPipeline* GetRenderPipeline(const std::string& name) const;

		MainCamera* GetSceneCamera() { return SceneCamera; }

	private:
		template<typename T> void OnComponentAdded(Entity entity, T& component);

		std::unordered_map<std::string, ScopePointer<RenderPipeline>> RenderPipelines{};

		entt::registry Registry;
		INT32 Width = 0;
		INT32 Height = 0;

		MainCamera* SceneCamera;
		WorldSettings SceneSettings;
	};
}
