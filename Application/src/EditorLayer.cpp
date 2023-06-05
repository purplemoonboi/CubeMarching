#include "EditorLayer.h"

#include "Framework/Scene/Scene.h"
#include "Framework/Core/Input/Input.h"
#include "Framework/Core/Compute/ComputeInstruction.h"
#include "Framework/Renderer/Textures/RenderTarget.h"
#include "Framework/Renderer/Textures/Texture.h"
#include "Framework/Scene/WorldSettings.h"

#include "Isosurface/VoxelWorldConstantExpressions.h"

#include <../ImGui/imgui.h>
#include <../ImGui/imgui_internal.h>

namespace Foundation
{
    EditorLayer::EditorLayer()
        :
        Layer(L"Scene Editor"),
		TimerManager(nullptr)
    {
        Scene = CreateRef<class Scene>("Test Scene");
        TimerManager = Application::Get()->GetApplicationTimeManager();

        

    }

    EditorLayer::~EditorLayer()
    {
        

    }

    void EditorLayer::OnAttach()
    {
        Renderer = RenderInstruction::GetApiPtr();
        APP_ASSERT(!Renderer);

        ComputeInstruction::Init(Renderer->GetGraphicsContext());
        const auto csApi = ComputeInstruction::GetComputeApi();

        RenderInstruction::OnBeginResourceCreation();



        RenderInstruction::OnEndResourceCreation();

        MainCamera* mc = Scene->GetSceneCamera();
        mc->SetPosition({0, 0, 0});

        SceneHierarchy.SetContext(Scene);
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate(const DeltaTime& deltaTime)
    {
        MainCamera* mc = Scene->GetSceneCamera();

        if(IsViewportFocused)
        {
            
            const auto camera = Scene->GetSceneCamera();

            if (Input::IsKeyPressed(KEY_E))
            {
                camera->Ascend(deltaTime);
            }
            if (Input::IsKeyPressed(KEY_Q))
            {
                camera->Ascend(-deltaTime);
            }
            if (Input::IsKeyPressed(KEY_W))
            {
                camera->Walk(deltaTime);
            }
            if (Input::IsKeyPressed(KEY_S))
            {
                camera->Walk(-deltaTime);
            }
            if (Input::IsKeyPressed(KEY_A))
            {
                camera->Strafe(-deltaTime);
            }
            if (Input::IsKeyPressed(KEY_D))
            {
                camera->Strafe(deltaTime);
            }

        
        }
        

        auto rt = Renderer->GetFrameBuffer();

        if(ViewportSize.x > 0 && ViewportSize.x < 8196 && ViewportSize.y > 0 && ViewportSize.y < 8196)
        {
            if (rt->GetWidth() != (INT32)ViewportSize.x || rt->GetHeight() != (INT32)ViewportSize.y)
            {
                mc->SetAspectRatio(ViewportSize.x, ViewportSize.y);
                mc->RecalculateAspectRatio(ViewportSize.x, ViewportSize.y);
                FrameBufferSpecifications fb{};
                fb.Width = ViewportSize.x;
                fb.Height = ViewportSize.y;
                //rt->OnResizeFrameBuffer(fb);
            }
        }

    }

    static bool wireframe = false;

    void EditorLayer::OnRender(const DeltaTime& timer)
    {

        Scene->OnRender(timer.GetSeconds(), TimerManager->TimeElapsed(), Settings, wireframe);


    }

    void EditorLayer::OnImGuiRender()
    {

            static bool dockspace_open = true;
            static bool opt_fullscreen = true;
            static bool opt_padding = true;
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

            // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
            // because it would be confusing to have two docking targets within each others.
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            if (opt_fullscreen)
            {
                const ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->WorkPos);
                ImGui::SetNextWindowSize(viewport->WorkSize);
                ImGui::SetNextWindowViewport(viewport->ID);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            }
            else
            {
                dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
            }

            // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
            // and handle the pass-thru hole, so we ask Begin() to not render a background.
            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;

            if (!opt_padding)
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("DockSpace", &dockspace_open, window_flags);//BEGIN DOCKSPACE
            if (!opt_padding)
                ImGui::PopStyleVar();

            if (opt_fullscreen)
                ImGui::PopStyleVar(2);



            ImGuiIO& io = ImGui::GetIO();
            ImGuiStyle& style = ImGui::GetStyle();
            const float minWindowSize = style.WindowMinSize.x;
            style.WindowMinSize.x = 360.0f;
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }

            style.WindowMinSize.x = minWindowSize;

            //MENU BAR - Menu dropdown
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    // Disabling fullscreen would allow the window to be moved to the front of other windows,
                    // which we can't undo at the moment without finer window depth/z control.
                    if (ImGui::MenuItem("New", "Ctrl+N"))
                    {
                        //NewScene();
                    }

                    if (ImGui::MenuItem("Open...", "Crtl+O"))
                    {
                       //OpenScene();
                    }

                    if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                    {
                        //SaveAs();
                    }

                    ImGui::Separator();

                    ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);

                    if (ImGui::MenuItem("Close", NULL, false))
                        dockspace_open = false;
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }


            auto* cam = Scene->GetSceneCamera();
            static float speed = cam->GetCameraFlySpeed();
            static float angleSpeed = cam->GetCameraAngularSpeed();
            {
                ImGui::Begin("Scene Camera");
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::DragFloat("Speed", &speed, 1.0f, 1.0f, 100.0f);
                ImGui::Spacing();
                ImGui::DragFloat("Angular Speed", &angleSpeed, 1.0f, 1.0f, 10.0f);
                ImGui::Spacing();
                ImGui::End();
            }
            cam->SetCameraFlySpeed(speed);
            cam->SetCameraAngularSpeed(angleSpeed);

            static float dir[3] = { 0, -1, 0 };
            static float amb[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
            //LIGHT
            {
                ImGui::Begin("Lighting Settings");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::DragFloat3("Sun Direction", dir, 0.1, -1.0f, 1.0f);
                ImGui::Spacing();
            	ImGui::Separator();
                ImGui::Spacing();
                ImGui::ColorPicker4("Ambiance Override", amb);


                ImGui::End();
                Settings.SunDirection[0] = dir[0];
                Settings.SunDirection[1] = dir[1];
                Settings.SunDirection[2] = dir[2];

                Settings.SunColour[0] = amb[0];
                Settings.SunColour[1] = amb[1];
                Settings.SunColour[2] = amb[2];
                Settings.SunColour[3] = amb[3];

            }

            

            ImGui::Spacing();



            //PROFILING - The profile window
            {
                ImGui::Begin("Terrain Statistics");
                ImGui::Text("Mesh Statistics:");
                auto stats = Renderer3D::GetProfileData();
                ImGui::Text("Vert Count : %d", stats.VertexCount);
                ImGui::Text("Tri  Count : %d", stats.TriCount);
                ImGui::End();
            }
         
            //VIEWPORT - This is where you would want to draw scene gizmos
            {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::Begin("Viewport");

                IsViewportFocused = ImGui::IsWindowFocused();
                IsViewportHovered = ImGui::IsWindowHovered();
                Application::Get()->GetImGuiLayer()->SetBlockEvents(!IsViewportFocused || !IsViewportHovered);


                ImVec2 viewport_region = ImGui::GetContentRegionAvail();
                DirectX::XMFLOAT2 viewportRegionXF2 = { viewport_region.x, viewport_region.y };
                if (ViewportSize.x != viewportRegionXF2.x || ViewportSize.y != viewportRegionXF2.y)
                {
                    ViewportSize = { viewport_region.x, viewport_region.y };
                }

                
                auto rt = RenderInstruction::GetApiPtr()->GetSceneAlbedoTexture();
                ImGui::Image((ImTextureID)rt->GetTexture(), ImVec2(ViewportSize.x, ViewportSize.y), {0,0}, {1,1});


                ImGui::End();
                ImGui::PopStyleVar();
            }



            SceneHierarchy.OnImGuiRender();

            //END DOCKSPACE
            ImGui::End();
        

        ImGui::Render();
    }

    void EditorLayer::OnEvent(Event& event)
    {
        /** process mouse and keyboard events */
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<WindowResizeEvent>(BIND_DELEGATE(EditorLayer::OnWindowResize));
        dispatcher.Dispatch<MouseMovedEvent>(BIND_DELEGATE(EditorLayer::OnMouseMove));
        dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_DELEGATE(EditorLayer::OnMouseDown));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_DELEGATE(EditorLayer::OnMouseUp));
    }

    bool EditorLayer::OnWindowResize(WindowResizeEvent& wndResize)
    {
        Scene->GetSceneCamera()->RecalculateAspectRatio(ViewportSize.x, ViewportSize.y);
        return false;
    }


    bool EditorLayer::OnMouseDown(MouseButtonPressedEvent& mEvent)
    {
        if ((mEvent.GetMouseButton() & MK_LBUTTON) != 0)
        {
            LeftMButton = true;
            MouseDownX = (float)mEvent.GetMouseX();
            MouseDownY = (float)mEvent.GetMouseY();
            auto camera = Scene->GetSceneCamera();

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

        if(LeftMButton && IsViewportFocused)
        {
            auto camera = Scene->GetSceneCamera();
            camera->Pitch(CurrentMouseY);
        	camera->RotateY(CurrentMouseX);
        }
   

        return false;
    }


}
