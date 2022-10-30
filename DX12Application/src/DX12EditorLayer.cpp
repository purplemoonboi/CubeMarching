#include "DX12EditorLayer.h"

namespace DX12Framework
{
    DX12EditorLayer::DX12EditorLayer()
        :
        Layer(L"Scene Editor")
    {
        World = new Scene("Test Scene");
    }

    DX12EditorLayer::~DX12EditorLayer()
    {
    }

    void DX12EditorLayer::OnAttach()
    {
    }

    void DX12EditorLayer::OnDetach()
    {
    }

    void DX12EditorLayer::OnUpdate(DeltaTime deltaTime)
    {
        //User input

        World->OnUpdate(deltaTime);
    }

    void DX12EditorLayer::OnRender()
    {
        World->OnRender();
    }

    void DX12EditorLayer::OnImGuiRender()
    {
    }

    void DX12EditorLayer::OnEvent(Event& event)
    {

        /** process mouse and keyboard events */

        EventDispatcher dispatcher(event);

         //   dispatcher.Dispatch<WindowResizeEvent>(BIND_DELEGATE(DX12EditorLayer::OnWindowResize));
        //   dispatcher.Dispatch<KeyPressedEvent>(BIND_DELEGATE(OnKeyPressed));
     //   dispatcher.Dispatch<KeyPressedEvent>(BIND_DELEGATE(DX12EditorLayer::OnKeyReleased));
     //
     //   dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_DELEGATE(DX12EditorLayer::OnMouseDown));
     //   dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_DELEGATE(DX12EditorLayer::OnMouseUp));
     //   dispatcher.Dispatch<MouseMovedEvent>(BIND_DELEGATE(DX12EditorLayer::OnMouseMove));

    }

    void DX12EditorLayer::OnWindowResize(WindowResizeEvent& wndResize)
    {
        World->SceneCamera->RecalculateAspectRatio(wndResize.GetWidth(), wndResize.GetHeight());
    }

    void DX12EditorLayer::OnKeyPressed(KeyPressedEvent& keyEvent)
    {
    }

    void DX12EditorLayer::OnKeyReleased(KeyReleasedEvent& keyEvent)
    {
    }

    void DX12EditorLayer::OnMouseDown(MouseButtonPressedEvent& mEvent)
    {

    }

    void DX12EditorLayer::OnMouseUp(MouseButtonPressedEvent& mEvent)
    {

    }

    void DX12EditorLayer::OnMouseMove(MouseMovedEvent& mEvent)
    {

    }
}
