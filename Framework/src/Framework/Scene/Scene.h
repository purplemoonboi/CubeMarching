#pragma once
#include "Framework/Core/Core.h"

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

		void OnUpdate(const float deltaTime);
		void OnRender();

		MainCamera* GetSceneCamera() const { return SceneCamera; }

	private:
		MainCamera* SceneCamera;
	};
}
