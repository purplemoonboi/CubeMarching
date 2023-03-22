#pragma once
#include "entt.hpp"
#include "Framework/Core/Core.h"
#include "Framework/Core/Time/DeltaTime.h"

namespace Editor
{
	class SceneHierarchyPanel;
}

namespace Engine
{
	struct WorldSettings;
	class MainCamera;

	class Entity;
	
	class Scene
	{
		friend class Entity;
		friend class SceneSerializer;
		friend class Editor::SceneHierarchyPanel;
	public:
		Scene() = delete;
		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene(const std::string& name);

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnUpdate(const float deltaTime, const float elapsedTime);
		void OnRender(const float deltaTime, const float elapsedTime, const WorldSettings& settings, bool wireframe = false) const;

		MainCamera* GetSceneCamera() { return SceneCamera; }

	private:

		template<typename T> void OnComponentAdded(Entity entity, T& component);


		entt::registry Registry;
		int Width = 0;
		int Height = 0;
		MainCamera* SceneCamera;
	};
}
