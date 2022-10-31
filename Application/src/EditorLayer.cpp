#include "EditorLayer.h"

#include "Framework/Engine.h"

namespace Engine
{
    EditorLayer::EditorLayer()
        :
        Layer(L"Scene Editor")
    {
        World = new Scene("Test Scene");
    }

    EditorLayer::~EditorLayer()
    {
    }

    void EditorLayer::OnAttach()
    {
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate(DeltaTime deltaTime)
    {
        //User input

        World->OnUpdate(deltaTime);
    }

    void EditorLayer::OnRender()
    {
        World->OnRender();
    }

    void EditorLayer::OnImGuiRender()
    {
    }

    void EditorLayer::OnEvent(Event& event)
    {

        /** process mouse and keyboard events */

        EventDispatcher dispatcher(event);

    /*	dispatcher.Dispatch<WindowResizeEvent>(BIND_DELEGATE(EditorLayer::OnWindowResize));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_DELEGATE(EditorLayer::OnKeyPressed));
        dispatcher.Dispatch<KeyPressedEvent>(BIND_DELEGATE(EditorLayer::OnKeyReleased));
     
        dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_DELEGATE(EditorLayer::OnMouseDown));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_DELEGATE(EditorLayer::OnMouseUp));
        dispatcher.Dispatch<MouseMovedEvent>(BIND_DELEGATE(EditorLayer::OnMouseMove));*/

    }

    void EditorLayer::OnWindowResize(WindowResizeEvent& wndResize)
    {
        World->GetSceneCamera()->RecalculateAspectRatio(wndResize.GetWidth(), wndResize.GetHeight());
    }

    void EditorLayer::OnKeyPressed(KeyPressedEvent& keyEvent)
    {
    }

    void EditorLayer::OnKeyReleased(KeyReleasedEvent& keyEvent)
    {
    }

    void EditorLayer::OnMouseDown(MouseButtonPressedEvent& mEvent)
    {

    }

    void EditorLayer::OnMouseUp(MouseButtonPressedEvent& mEvent)
    {

    }

    void EditorLayer::OnMouseMove(MouseMovedEvent& mEvent)
    {

    }
}
