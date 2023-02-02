#pragma once
#include "Framework/Core/Core.h"
#include "Framework/Scene/Scene.h"

namespace Editor
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(Engine::RefPointer<Engine::Scene> sceneContext);

		void SetContext(const Engine::RefPointer<Engine::Scene>& context);


	private:
		Engine::RefPointer<Engine::Scene> CurrentScene;
	};
}