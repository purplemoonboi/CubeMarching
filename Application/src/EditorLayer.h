#pragma once
#include <Framework/Engine.h>

#include "Panels/SceneHierarchyPanel.h"


namespace Foundation::Editor
{
	
	using namespace Graphics;
	

	class EditorLayer : public Layer
	{
	public:
		HMODULE GpuCaptureModule;
		HMODULE GpuTimingModule;
	public:

		EditorLayer();
		virtual ~EditorLayer() override;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(AppTimeManager* time) override;
		void OnRender(AppTimeManager* time) override;
		void OnImGuiRender() override;
		void OnEvent(Event& event) override;

	private:
		bool OnWindowResize(WindowResizeEvent& wndResize);
		bool OnMouseDown(MouseButtonPressedEvent& mEvent);
		bool OnMouseUp(MouseButtonReleasedEvent& mEvent);
		bool OnMouseMove(MouseMovedEvent& mEvent);

	private:

		RendererAPI* Renderer = nullptr;

	private:
		SceneHierarchyPanel SceneHierarchy;


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


