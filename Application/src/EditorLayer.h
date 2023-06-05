#pragma once
#include <Framework/Scene/WorldSettings.h>
#include <Framework/Engine.h>

#include <Framework/Core/Time/DeltaTime.h>
#include <Framework/Core/Events/MouseEvent.h>
#include <Framework/Core/Events/AppEvents.h>
#include <Framework/Core/Events/KeyEvent.h>



#include "Panels/SceneHierarchyPanel.h"




namespace Foundation
{

	class EditorLayer : public Foundation::Layer
	{
	public:
		HMODULE GpuCaptureModule;
		HMODULE GpuTimingModule;
	public:

		EditorLayer();
		virtual ~EditorLayer() override;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(const DeltaTime& timer) override;
		void OnRender(const DeltaTime& timer) override;
		void OnImGuiRender() override;
		void OnEvent(Event& event) override;

	private:
		bool OnWindowResize(WindowResizeEvent& wndResize);
		bool OnMouseDown(MouseButtonPressedEvent& mEvent);
		bool OnMouseUp(MouseButtonReleasedEvent& mEvent);
		bool OnMouseMove(MouseMovedEvent& mEvent);

	private:
		WorldSettings Settings;

		RendererAPI* Renderer = nullptr;

	private:


		Editor::SceneHierarchyPanel SceneHierarchy;


		bool IsViewportFocused;
		bool IsViewportHovered;

		DirectX::XMFLOAT2 ViewportSize;

		float CurrentMouseX = 0;
		float CurrentMouseY = 0;
		float LastMouseX = 0.f;
		float LastMouseY = 0.f;
		float MouseDownX = 0;
		float MouseDownY = 0;



		bool LeftMButton = false;
		bool RightMButton = false;
		bool MouseMoved = false;

		AppTimeManager* TimerManager;
		RefPointer<Scene> Scene;
	};

}


