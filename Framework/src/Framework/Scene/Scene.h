#pragma once
#include "Framework/Core/Core.h"
#include "Framework/Core/Time/AppTimeManager.h"

namespace Engine
{
	class MainCamera;

	class Scene
	{
	public:
		Scene() = delete;
		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene(const std::string& name);

		void OnUpdate(const  AppTimeManager& timer);
		void OnRender(const  AppTimeManager& timer);

		MainCamera* GetSceneCamera() const { return SceneCamera; }

	private:
		MainCamera* SceneCamera;
	};
}
