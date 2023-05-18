#include "SceneHierarchyPanel.h"

#include <../imgui/imgui.h>
#include <../imgui/imgui_internal.h>

namespace Editor
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Foundation::RefPointer<Foundation::Scene>& context)
		:
		ActiveScene(context)
	{
	}

	void SceneHierarchyPanel::SetContext(const Foundation::RefPointer<Foundation::Scene>& context)
	{
		ActiveScene = context;
		SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		//Render the scene hierarchy panel.
		ImGui::Begin("Scene Hierarchy");

		//For each entity in the scene, draw a node in the panel.
		ActiveScene->Registry.each([&](auto entity_id)
			{
				Foundation::Entity entity{ entity_id, ActiveScene.get() };
				DrawEntityNode(entity);
			});

		//Clear the selected context if we click an empty space.
		if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(0))
		{
			SelectionContext = {};
		}


		//Pop up menu when right-click on a blank space.
		if (ImGui::BeginPopupContextWindow("Create", 1))
		{
			if (ImGui::MenuItem("Create an empty entity"))
			{
				ActiveScene->CreateEntity("Empty Entity");
			}

			ImGui::EndPopup();
		}

		ImGui::End();

		//Render the entity properties panel.
		ImGui::Begin("Properties");
		if (SelectionContext)
		{
			/// <summary>
			/// Render all the components for the currently selected entity.
			/// Additionally render buttons and menus to add/remove components.
			/// </summary>

			DrawComponents(SelectionContext);
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Foundation::Entity entity)
	{
		auto& entityTag = entity.GetComponent<Foundation::TagComponent>().tag;


		ImGuiTreeNodeFlags flags = ((SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		//This node is a tree which can be expanded in the panel.
		bool expanded = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, entityTag.c_str());

		//If *this* node is clicked, set it to the current selected context.
		if (ImGui::IsItemClicked())
		{
			SelectionContext = entity;
		}

		//If right clicked, create a popup menu. 
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
			{
				entityDeleted = true;
			}

			ImGui::EndPopup();
		}

		if (expanded)
		{
			ImGui::TreePop();
		}

		//Deffered deletion to avoid any issues.
		if (entityDeleted)
		{
			//Remove the entity from registery.
			ActiveScene->DestroyEntity(entity);
			if (SelectionContext == entity)
			{
				//Clear the selected context.
				SelectionContext = {};
			}
		}
	}

	template <typename T, typename UIFunction> static void DrawComponent(const std::string& name, Foundation::Entity entity, UIFunction uiFunction)
	{
		//Set the tree node flags.
		const ImGuiTreeNodeFlags treeNodeFlags =
			ImGuiTreeNodeFlags_DefaultOpen |
			ImGuiTreeNodeFlags_AllowItemOverlap |
			ImGuiTreeNodeFlags_Framed |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_FramePadding;

		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();

			const ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			ImGui::Separator();
			bool opened = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);

			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeComponent = true;
				}
				ImGui::EndPopup();
			}

			if (opened)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
			{
				entity.RemoveComponent<T>();
			}
		}
	}

	static void DrawVec3Control(const std::string& label, DirectX::XMFLOAT3& values, float resetValue = 0.f, float columnWidth = 100.f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		//Two widgets adjacent to each other. i.e (	Text() | DragFloat() )
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);

		ImGui::Text(label.c_str());

		//Evrything below affects the adjacent widget.
		ImGui::NextColumn();


		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
		}
		ImGui::PopStyleColor(3);
		ImGui::PopFont();

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
		}
		ImGui::PopStyleColor(3);
		ImGui::PopFont();

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
		{
			values.z = resetValue;
		}
		ImGui::PopStyleColor(3);
		ImGui::PopFont();

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		//Pop the style parameters.
		ImGui::PopStyleVar();
		ImGui::Columns(1);

		//Pop the unique ID for this block of parameters.
		ImGui::PopID();
	}

	void SceneHierarchyPanel::DrawComponents(Foundation::Entity entity)
	{
		//Draw each component linked to *this* entity. 


		if (entity.HasComponent<Foundation::TagComponent>())
		{
			auto& tag = entity.GetComponent<Foundation::TagComponent>().tag;
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);


		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponent");
		}

		if (ImGui::BeginPopup("AddComponent"))
		{
			if (ImGui::MenuItem("Camera"))
			{
				SelectionContext.AddComponent<Foundation::CameraComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Sprite Renderer"))
			{
				SelectionContext.AddComponent<Foundation::SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("C++ Script"))
			{
				//SelectionContext.AddComponent<NativeScriptComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Lua Script"))
			{
				//SelectionContext.AddComponent<NativeScriptComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		//Draw components.

			DrawComponent<Foundation::TransformComponent>("Transform", entity, [](auto& component)
			{
				DrawVec3Control("Translation", component.Translation);
				DrawVec3Control("Rotation", component.Rotation);
				DrawVec3Control("Scale", component.Scale, 1.0f);
			});

			DrawComponent<Foundation::CameraComponent>("Camera", entity, [](auto& component)
			{
				Foundation::MainCamera& camera = component.Camera;

				ImGui::Checkbox("Primary", &component.Primary);

				const char* projectionTypeStr[] =
				{
					"Perspective",
					"Orthographic"
				};

				const char* currentProjectionType = projectionTypeStr[(uint32_t)component.Camera.GetProjectionType()];

				if (ImGui::BeginCombo("Projection", currentProjectionType))
				{
					for (int i = 0; i < 2; ++i)
					{
						bool isSelected = currentProjectionType == projectionTypeStr[i];
						if (ImGui::Selectable(projectionTypeStr[i], isSelected))
						{
							currentProjectionType = projectionTypeStr[i];
							camera.SetProjectionType((Foundation::MainCamera::ProjectionType)i);
						}

						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}

					}
					ImGui::EndCombo();
				}

				if (component.Camera.GetProjectionType() == (INT)Foundation::MainCamera::ProjectionType::Perspective)
				{
					float perspectiveFov = DirectX::XMConvertToDegrees(camera.GetPerspectiveFOV());
					if (ImGui::DragFloat("Fov", &perspectiveFov))
					{
						camera.SetPerspectiveFOV(DirectX::XMConvertToRadians(perspectiveFov));
					}
					float perspectiveNear = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near Clip", &perspectiveNear))
					{
						camera.SetPerspectiveNearClip(perspectiveNear);
					}
					float perspectiveFar = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far Clip", &perspectiveFar))
					{
						camera.SetPerspectiveFarClip(perspectiveFar);
					}
				}

				if (component.Camera.GetProjectionType() == (INT)Foundation::MainCamera::ProjectionType::Orthographic)
				{
					float orthoSize = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &orthoSize))
					{
						camera.SetOrthographicSize(orthoSize);
					}
					float orthoNear = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near Clip", &orthoNear))
					{
						camera.SetOrthographicNearClip(orthoNear);
					}
					float orthoFar = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far Clip", &orthoFar))
					{
						camera.SetOrthographicFarClip(orthoFar);
					}

					ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
				}
			});

		DrawComponent<Foundation::SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
		{
				float col[] = { component.Colour.x, component.Colour.y, component.Colour.z, component.Colour.w };
			ImGui::ColorEdit4("Colour", col);
		});

		DrawComponent<Foundation::MeshComponent>("Mesh Component", entity, [](auto& component)
		{
				const char* name = component.Mesh->GetName().c_str();
				ImGui::Text("Name", name);

				bool wire = component.WireFrame;
				ImGui::Checkbox("WireFrame", &wire);


		});
	}
}
