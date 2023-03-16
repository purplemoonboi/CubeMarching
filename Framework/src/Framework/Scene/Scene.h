#pragma once
#include "Framework/Core/Core.h"
#include "Framework/Core/Time/DeltaTime.h"

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

		void OnUpdate(const float deltaTime, const float elapsedTime);
		void OnRender(const float deltaTime, const float elapsedTime) const;

		MainCamera* GetSceneCamera() { return SceneCamera; }

	private:
		MainCamera* SceneCamera;
	};
}
