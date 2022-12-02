#include "EditorLayer.h"

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
        MainCamera* mc = World->GetSceneCamera();

        

        //User input
        if(LeftMButton)
        {
            // Make each pixel correspond to a quarter of a degree.
            float dx = DirectX::XMConvertToRadians(0.25f * MouseDownX - DeltaMouseX);
            float dy = DirectX::XMConvertToRadians(0.25f * MouseDownY - DeltaMouseY);


            // Update angles based on input to orbit camera around box.
            mc->UpdateCameraZenith(dx, timer);
            mc->UpdateCamerasAzimuth(dy, timer);
        }
        else if(RightMButton)
        {
            // Make each pixel correspond to 0.2 unit in the scene.
            float dx = 0.05f * MouseDownX - MouseLastX;
            float dy = 0.05f * MouseDownY - MouseLastY;


            // Update the camera radius based on input.
           mc->UpdateCamerasDistanceToTarget(dx - dy, timer);
        }

       
        MouseLastX = DeltaMouseX;
        MouseLastY = DeltaMouseY;

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

        dispatcher.Dispatch<WindowResizeEvent>(BIND_DELEGATE(EditorLayer::OnWindowResize));

        //TODO: This will need to become a native script but will do for now.

        dispatcher.Dispatch<MouseMovedEvent>(BIND_DELEGATE(EditorLayer::OnMouseMove));

        dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_DELEGATE(EditorLayer::OnMouseDown));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_DELEGATE(EditorLayer::OnMouseUp));

        //TODO: End ToDo.

    }

    bool EditorLayer::OnWindowResize(WindowResizeEvent& wndResize)
    {
        World->GetSceneCamera()->RecalculateAspectRatio(wndResize.GetWidth(), wndResize.GetHeight());


        return false;
    }


    bool EditorLayer::OnMouseDown(MouseButtonPressedEvent& mEvent)
    {
        if ((mEvent.GetMouseButton() & MK_LBUTTON) != 0)
        {
            LeftMButton = true;
            MouseDownX = (float)mEvent.GetMouseX();
            MouseDownY = (float)mEvent.GetMouseY();
            CORE_TRACE("Button down");
        }
        if ((mEvent.GetMouseButton() & MK_RBUTTON) != 0)
        {
            MouseDownX = (float)mEvent.GetMouseX();
            MouseDownY = (float)mEvent.GetMouseY();
            RightMButton = true;
        }

        return false;
    }

    bool EditorLayer::OnMouseUp(MouseButtonReleasedEvent& mEvent)
    {
        if (~(mEvent.GetMouseButton() & MK_LBUTTON) != 0)
        {
            LeftMButton = false;
        }
        if (~(mEvent.GetMouseButton() & MK_RBUTTON) != 0)
        {
            RightMButton = false;
        }
        return false;
    }

    bool EditorLayer::OnMouseMove(MouseMovedEvent& mEvent)
    {
        DeltaMouseX = mEvent.GetXCoordinate();
        DeltaMouseY = mEvent.GetYCoordinate();

        return false;
    }

  
}
