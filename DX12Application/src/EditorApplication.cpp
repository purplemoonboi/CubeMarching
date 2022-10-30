#include "EditorApplication.h"

namespace DX12Framework
{
    EditorApplication::EditorApplication()
        :
        Layer(L"Scene Editor")
    {
        World = new Scene("Test Scene");
    }

    EditorApplication::~EditorApplication()
    {
    }

    void EditorApplication::OnAttach()
    {
    }

    void EditorApplication::OnDetach()
    {
    }

    void EditorApplication::OnUpdate(DeltaTime deltaTime)
    {
        //User input

        World->OnUpdate(deltaTime);
    }

    void EditorApplication::OnRender()
    {
        World->OnRender();
    }

    void EditorApplication::OnImGuiRender()
    {
    }

    void EditorApplication::OnEvent(Event& event)
    {

        /** process mouse and keyboard events */

        EventDispatcher dispatcher(event);

    	dispatcher.Dispatch<WindowResizeEvent>(BIND_DELEGATE(EditorApplication::OnWindowResize));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_DELEGATE(EditorApplication::OnKeyPressed));
        dispatcher.Dispatch<KeyPressedEvent>(BIND_DELEGATE(EditorApplication::OnKeyReleased));
     
        dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_DELEGATE(EditorApplication::OnMouseDown));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_DELEGATE(EditorApplication::OnMouseUp));
        dispatcher.Dispatch<MouseMovedEvent>(BIND_DELEGATE(EditorApplication::OnMouseMove));

    }

    void EditorApplication::OnWindowResize(WindowResizeEvent& wndResize)
    {
        World->SceneCamera->RecalculateAspectRatio(wndResize.GetWidth(), wndResize.GetHeight());
    }

    void EditorApplication::OnKeyPressed(KeyPressedEvent& keyEvent)
    {
    }

    void EditorApplication::OnKeyReleased(KeyReleasedEvent& keyEvent)
    {
    }

    void EditorApplication::OnMouseDown(MouseButtonPressedEvent& mEvent)
    {

    }

    void EditorApplication::OnMouseUp(MouseButtonPressedEvent& mEvent)
    {

    }

    void EditorApplication::OnMouseMove(MouseMovedEvent& mEvent)
    {

    }
}
