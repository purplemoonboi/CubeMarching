#pragma once
#include "Framework/Core/Core.h"
#include "Framework/Scene/Scene.h"
#include "Framework/Scene/Entity.h"
#include "Framework/Scene/Components.h"

namespace Editor
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Engine::RefPointer<Engine::Scene>& context);

		// @brief Set the current scene's context.
		// @param[in] Takes a reference to a shared pointer to a scene object.
		void SetContext(const Engine::RefPointer<Engine::Scene>& context);

		// @brief Renders all the ImGui panels.
		void OnImGuiRender();

	public:
		Engine::Entity GetSelectedEntity() const { return SelectionContext; }

	private:

		// @brief Renders entity node in the scene's hierarchy panel.
		// @param[in] Takes an Entity as the argument.
		void DrawEntityNode(Engine::Entity entity);

		// @brief Renders all the components currently tied to the entity.
		// @param[in] Takes an Entity as the argument.
		void DrawComponents(Engine::Entity selected_context);

	private:
		Engine::RefPointer<Engine::Scene> ActiveScene;
		Engine::Entity SelectionContext;
	};
}