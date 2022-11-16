#pragma once
#include <Framework/Engine.h>
#include "Framework/Core/Time/DeltaTime.h"

namespace Engine
{

	class EditorLayer : public Layer
	{
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

		void OnWindowResize(WindowResizeEvent& wndResize);

		void OnKeyPressed(KeyPressedEvent& keyEvent);
		void OnKeyReleased(KeyReleasedEvent& keyEvent);

		void OnMouseDown(MouseButtonPressedEvent& mEvent);
		void OnMouseUp(MouseButtonPressedEvent& mEvent);
		void OnMouseMove(MouseMovedEvent& mEvent);

		Scene* World;
	};

}


