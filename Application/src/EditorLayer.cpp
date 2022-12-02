#include "EditorLayer.h"
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

    void EditorLayer::OnUpdate(const DeltaTime& deltaTime)
    {
        MainCamera* mc = World->GetSceneCamera();

        const float elapsedTime = Application::Get()->GetApplicationTimeManager()->TimeElapsed();


        if(CurrentMouseX != MouseLastX)
        {
            Remap(CurrentMouseX, 0, 1920, -1, 1);
            Remap(CurrentMouseY, 0, 1080, -1, 1);
        }
       

        //User input
        if (MouseMoved && LeftMButton)
        {
            // Make each pixel correspond to a quarter of a degree.
            float dx = DirectX::XMConvertToRadians(90.f * CurrentMouseX);
            float dy = DirectX::XMConvertToRadians(90.f * CurrentMouseY);


            // Update angles based on input to orbit camera around box.
            mc->UpdateCamerasAzimuth(dx, deltaTime);
            mc->UpdateCameraZenith(dy, deltaTime);
        }
        if(RightMButton)
        {
            // Make each pixel correspond to 0.2 unit in the scene.
            float dx = 50.f * CurrentMouseX;
            float dy = 50.f * CurrentMouseY;


            // Update the camera radius based on input.
           mc->UpdateCamerasDistanceToTarget((dx - dy), deltaTime);
        }

        CORE_TRACE("Delta Mouse {0}, {1}", CurrentMouseX, CurrentMouseY);

        MouseLastX = CurrentMouseX;
        MouseLastY = CurrentMouseY;


        World->OnUpdate(deltaTime.GetSeconds(), elapsedTime);
    }

    void EditorLayer::OnRender(const DeltaTime& timer)
    {
        const float elapsedTime = Application::Get()->GetApplicationTimeManager()->TimeElapsed();

        World->OnRender(timer.GetSeconds(), elapsedTime);
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
        CurrentMouseX = mEvent.GetXCoordinate();
        CurrentMouseY = mEvent.GetYCoordinate();
        MouseMoved = true;

        return false;
    }

    void EditorLayer::Remap(float& x, float clx, float cmx, float nlx, float nmx)
    {
        x = nlx + (x - clx) * (nmx - nlx) / (cmx - clx);
    }
}
