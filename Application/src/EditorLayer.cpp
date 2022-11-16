#include "EditorLayer.h"

#include "Framework/Engine.h"
#include "Framework/Scene/Scene.h"

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

    void EditorLayer::OnUpdate(const DeltaTime& timer)
    {
        //User input


        World->OnUpdate(timer);
    }

    void EditorLayer::OnRender(const DeltaTime& timer)
    {
        World->OnRender(timer);
    }

    void EditorLayer::OnImGuiRender()
    {
    }

    void EditorLayer::OnEvent(Event& event)
    {

        /** process mouse and keyboard events */

        EventDispatcher dispatcher(event);

		//
  //  	dispatcher.Dispatch<WindowResizeEvent>(BIND_DELEGATE(EditorLayer::OnWindowResize));
		//dispatcher.Dispatch<KeyPressedEvent>(BIND_DELEGATE(EditorLayer::OnKeyPressed));
  //      dispatcher.Dispatch<KeyPressedEvent>(BIND_DELEGATE(EditorLayer::OnKeyReleased));
  //   
  //      dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_DELEGATE(EditorLayer::OnMouseDown));
  //      dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_DELEGATE(EditorLayer::OnMouseUp));
  //      dispatcher.Dispatch<MouseMovedEvent>(BIND_DELEGATE(EditorLayer::OnMouseMove));
        

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
