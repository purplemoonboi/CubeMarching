#include "Framework/cmpch.h"
#include "ImGuiLayer.h"
#include "Framework/Core/Application/Application.h"

#include <imgui.h>
#include "Framework/ImGui/ImGuiApi/ImGuiInstructions.h"

namespace Engine
{
	ImGuiLayer::ImGuiLayer()
		:
		Layer(L"ImGuiLayer"),
		BlockEvents(false),
		Time(0.f)
	{
	}

	ImGuiLayer::ImGuiLayer(ImGuiRenderingApi api)
		:
		Layer(L"ImGuiLayer"),
		BlockEvents(false),
		Time(0.f)
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::OnAttach()
	{

		ImGuiInstructions::InitialiseImGui();

		// Setup Dear ImGui style
		SetDarkThemeColours();
	}

	void ImGuiLayer::OnDetach()
	{
		ImGuiInstructions::CleanUp();
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
	}

	void ImGuiLayer::Begin() 
	{
		ImGuiInstructions::BeginRenderImpl();
		/*ImGui draw data here.*/

	}

	void ImGuiLayer::OnImGuiRender()
	{
	}

	void ImGuiLayer::End() 
	{
		ImGuiInstructions::EndRenderImpl();
	}

	void ImGuiLayer::SetDarkThemeColours()
	{
		auto& colours = ImGui::GetStyle().Colors;
		//WINDOW BACKGROUND
		colours[ImGuiCol_WindowBg] = ImVec4{ 0.1f,  0.105f,  0.11f,  1.0f };

		//HEADERS							 
		colours[ImGuiCol_Header] = ImVec4{ 0.205f,  0.205f,  0.21f,  1.0f };
		colours[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f,  0.305f,  0.31f,  1.0f };
		colours[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		//BUTTONS							 		   							    
		colours[ImGuiCol_Button] = ImVec4{ 0.2f,  0.2205f, 0.21f,  1.0f };
		colours[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f,  0.305f,  0.31f,  1.0f };
		colours[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		//FRAME BACKGROUND					 
		colours[ImGuiCol_FrameBg] = ImVec4{ 0.2f,  0.2205f, 0.21f,  1.0f };
		colours[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f,  0.305f,  0.31f,  1.0f };
		colours[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		//TABS
		colours[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colours[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colours[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colours[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colours[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f,  0.205f,  0.21f,  1.0f };

		//TITLE
		colours[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colours[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colours[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}

}

