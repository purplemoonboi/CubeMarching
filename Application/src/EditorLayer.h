#pragma once
#include <Framework/Engine.h>

namespace Engine
{

	class EditorLayer : public Layer
	{
	public:

		EditorLayer();
		virtual ~EditorLayer();

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(const AppTimeManager& timer) override;
		void OnRender(const AppTimeManager& timer)override;
		void OnImGuiRender() override;
		void OnEvent(Event& event) override;

	private:

		void OnWindowResize(WindowResizeEvent& wndResize);

		void OnKeyPressed(KeyPressedEvent& keyEvent);
		void OnKeyReleased(KeyReleasedEvent& keyEvent);

		void OnMouseDown(MouseButtonPressedEvent& mEvent);
		void OnMouseUp(MouseButtonPressedEvent& mEvent);
		void OnMouseMove(MouseMovedEvent& mEvent);

		Scene* World;
	};

}


