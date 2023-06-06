#pragma once
#include "Framework/Core/Core.h"
#include "Framework/Scene/Scene.h"
#include "Framework/Scene/Entity.h"
#include "Framework/Scene/Components.h"

namespace Foundation::Editor
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Foundation::RefPointer<Foundation::Scene>& context);

		// @brief Set the current scene's context.
		// @param[in] Takes a reference to a shared pointer to a scene object.
		void SetContext(const Foundation::RefPointer<Foundation::Scene>& context);

		// @brief Renders all the ImGui panels.
		void OnImGuiRender();

	public:
		Foundation::Entity GetSelectedEntity() const { return SelectionContext; }

	private:

		// @brief Renders entity node in the scene's hierarchy panel.
		// @param[in] Takes an Entity as the argument.
		void DrawEntityNode(Foundation::Entity entity);

		// @brief Renders all the components currently tied to the entity.
		// @param[in] Takes an Entity as the argument.
		void DrawComponents(Foundation::Entity selected_context);

	private:
		Foundation::RefPointer<Foundation::Scene> ActiveScene;
		Foundation::Entity SelectionContext;
	};
}