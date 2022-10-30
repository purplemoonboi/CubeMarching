#pragma once
#include <Framework/DX12Framework.h>

namespace DX12Framework
{

	class EditorApplication : public Layer
	{
	public:

		EditorApplication();
		virtual ~EditorApplication();

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(DeltaTime deltaTime) override;
		void OnRender()override;
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


