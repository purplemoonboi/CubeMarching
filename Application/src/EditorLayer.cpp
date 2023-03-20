#include "EditorLayer.h"

#include "Framework/Scene/Scene.h"
#include "Framework/Core/Input/Input.h"

#include <../ImGui/imgui.h>
#include <../ImGui/imgui_internal.h>

#include "Framework/Core/Compute/ComputeInstruction.h"
#include "Framework/Renderer/Textures/Texture.h"
#include "Framework/Scene/WorldSettings.h"
#include "Platform/DirectX12/Buffers/D3D12FrameBuffer.h"

#include "Isosurface/VoxelWorldConstantExpressions.h"
//change
namespace Engine
{
    EditorLayer::EditorLayer()
        :
        Layer(L"Scene Editor"),
		TimerManager(nullptr)
    {
        World = new Scene("Test Scene");
        TimerManager = Application::Get()->GetApplicationTimeManager();

        PerlinCompute = CreateScope<class DensityTextureGenerator>();

        MarchingCubes = CreateScope<class MarchingCubes>();
        //MarchingCubesHP = CreateScope<class MarchingCubesHP>

        DualContouring = CreateScope<class DualContouring>();
    	//DualContourSPO   = CreateScope < class DualContouringSPO >();

        Regen = true;
    }

    EditorLayer::~EditorLayer()
    {

        MarchingCubes.reset();
        MarchingCubes = nullptr;

        PerlinCompute.reset();
        PerlinCompute = nullptr;

    }

    void EditorLayer::OnAttach()
    {
        const auto api = RenderInstruction::GetApiPtr();

        ComputeInstruction::Init(api->GetGraphicsContext());
        const auto csApi = ComputeInstruction::GetComputeApi();

        RenderInstruction::ResetGraphicsCommandList();

        PerlinCompute->Init(csApi, api->GetMemoryManager());

        MarchingCubes->Init(csApi, api->GetMemoryManager());
        Renderer3D::CreateCustomMesh(MarchingCubes->GetVertices(), MarchingCubes->GetIndices(), "MarchingTerrain", Transform(0, 0, 0));

    	DualContouring->Init(csApi, api->GetMemoryManager());
        Renderer3D::CreateCustomMesh(DualContouring->GetVertices(), DualContouring->GetIndices(), "DualTerrain", Transform(20, 0, 0));

        //DualContourSPO->Init(csApi, api->GetMemoryManager());
        //MarchingCubesHP->Init(csApi, api->GetMemoryManager());

        ViewportTexture = Texture::Create(0, 1920U, 1080U, TextureFormat::RGBA_UINT_UNORM);

        RenderInstruction::ExecGraphicsCommandList();

        MainCamera* mc = World->GetSceneCamera();
        mc->SetPosition({0, 0, 0});
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate(const DeltaTime& deltaTime)
    {
        MainCamera* mc = World->GetSceneCamera();


        if(CurrentMouseX != MouseLastX)
        {
            Remap(CurrentMouseX, 0, 1920, -1, 1);
            Remap(CurrentMouseY, 0, 1080, -1, 1);
        }



        //User input
        if(!IsViewportFocused)
        {
            if (MouseMoved && LeftMButton)
            {

                // Make each pixel correspond to a quarter of a degree.
                float dx = 90.0f * CurrentMouseX;
                float dy = 90.0f * CurrentMouseY;


                // Update angles based on input to orbit camera around box.
                mc->UpdateCameraZenith(dy, deltaTime);

                mc->UpdateCamerasAzimuth(-dx, deltaTime);
            }
            if (RightMButton)
            {
                // Make each pixel correspond to 0.2 unit in the scene.
                float dx = 50.f * CurrentMouseX;
                float dy = 50.f * CurrentMouseY;


                // Update the camera radius based on input.
                mc->UpdateCamerasDistanceToTarget((dx - dy), deltaTime);
            }
            if (Input::IsKeyPressed(KEY_E))
            {
                auto camera = World->GetSceneCamera();

                XMFLOAT3 pos = camera->GetPosition();
                CORE_TRACE("Moving Camera Position {0},{1},{2}", pos.x, pos.y, pos.z);

                pos.y += 12.0f * deltaTime;


                camera->SetPosition(pos);
            }
            if (Input::IsKeyPressed(KEY_Q))
            {
                auto camera = World->GetSceneCamera();

                XMFLOAT3 pos = camera->GetPosition();
                CORE_TRACE("Moving Camera Position {0},{1},{2}", pos.x, pos.y, pos.z);

                pos.y -= 12.0f * deltaTime;


                camera->SetPosition(pos);
            }
            if (Input::IsKeyPressed(KEY_W))
            {
                auto camera = World->GetSceneCamera();

                XMFLOAT3 pos = camera->GetPosition();
                XMFLOAT3 forw = camera->GetForward();
                CORE_TRACE("Moving Camera Position {0},{1},{2}", pos.x, pos.y, pos.z);

                //pos.x -= forw.x * 5 * deltaTime;
                //pos.y -= forw.y * 5 * deltaTime;
                //pos.z -= forw.x * 5 * deltaTime;
                pos.z += 12.0f * deltaTime;

                camera->SetPosition(pos);
            }
            if (Input::IsKeyPressed(KEY_A))
            {
                auto camera = World->GetSceneCamera();

                XMFLOAT3 pos = camera->GetPosition();
                CORE_TRACE("Moving Camera Position {0},{1},{2}", pos.x, pos.y, pos.z);

                pos.x -= 12.0f * deltaTime;


                camera->SetPosition(pos);
            }
            if (Input::IsKeyPressed(KEY_D))
            {
                auto camera = World->GetSceneCamera();

                XMFLOAT3 pos = camera->GetPosition();
                CORE_TRACE("Moving Camera Position {0},{1},{2}", pos.x, pos.y, pos.z);

                pos.x += 12.0f * deltaTime;


                camera->SetPosition(pos);
            }
            if (Input::IsKeyPressed(KEY_S))
            {
                auto camera = World->GetSceneCamera();

                XMFLOAT3 pos = camera->GetPosition();
                CORE_TRACE("Moving Camera Position {0},{1},{2}", pos.x, pos.y, pos.z);

                pos.z -= 12.0f * deltaTime;


                camera->SetPosition(pos);
            }
        }
     


        MouseLastX = CurrentMouseX;
        MouseLastY = CurrentMouseY;


        World->OnUpdate(deltaTime.GetSeconds(), TimerManager->TimeElapsed());
    
        if (Regen)
        {
            Regen = false;

            PerlinSettings.ChunkCoord = { (float)0, 0, (float)0 };
            PerlinCompute->PerlinFBM(PerlinSettings);
            if(Smooth)
            {
                Smooth = false;
                PerlinCompute->Smooth(CsgOperationSettings);
            }

            /* polygonise the texture with marching cubes*/
            MarchingCubes->Dispatch(VoxelSettings, PerlinCompute->GetTexture());
            Renderer3D::RegenerateBuffers("MarchingTerrain", MarchingCubes->GetVertices(),
                MarchingCubes->GetIndices());

            /* polygonise the texture with dual contouring */
            /*DualContouring->Dispatch(VoxelSettings, DensityTextureGenerator->GetTexture());
            Renderer3D::RegenerateBuffers("DualTerrain", DualContouring->GetVertices(), 
                DualContouring->GetIndices());*/
            
        }
    }

    static bool wireframe = false;

    void EditorLayer::OnRender(const DeltaTime& timer)
    {

        World->OnRender(timer.GetSeconds(), TimerManager->TimeElapsed(), Settings, wireframe);
    }

    void EditorLayer::OnImGuiRender()
    {
        {
            // If we removed all the options we are showcasing, this demo would become:
            //     void ShowExampleAppDockSpace()
            //     {
            //         ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
            //     }
           

            static bool dockspace_open = true;
            static bool opt_fullscreen = false;
            static bool opt_padding = false;
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

            // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
            // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
            // all active windows docked into it will lose their parent and become undocked.
            // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
            // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
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

            {
                ImGui::Begin("DockSpace Settings");
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Checkbox("FullScreen", &opt_fullscreen);
                ImGui::End();
            }


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

            //VOXEL SETTINGS 
            {
                ImGui::Begin("Noise Settings");
                ImGui::Separator();
                ImGui::Spacing();
                float freq = PerlinSettings.Frequency;
                float gain = PerlinSettings.Gain;
                float groundHeight = PerlinSettings.GroundHeight;
                INT32 octaves = PerlinSettings.Octaves;
                INT32 smooth = CsgOperationSettings.Radius;

                if (ImGui::DragInt("Octaves", &octaves, 1, 1, 8))
                    Regen = true;

                if (ImGui::DragFloat("Frequency", &freq, 0.1f, 0.001f, 0.999f))
                    Regen = true;

                ImGui::Spacing();
                if (ImGui::DragFloat("Gain", &gain, 0.1f, 0.1f, 4.0f))
                    Regen = true;

                ImGui::Spacing();
                if (ImGui::DragFloat("Ground Height", &groundHeight, 0.1f, 2.0f, (float)ChunkHeight))
                    Regen = true;

                if (ImGui::DragInt("Smooth", &smooth, 1, 0, 8))
                    Smooth = true;

              

                ImGui::Separator();
                ImGui::End();
                PerlinSettings.Frequency = freq;
                PerlinSettings.Gain = gain;
                PerlinSettings.GroundHeight = groundHeight;
                PerlinSettings.Octaves = octaves;

                float isoVal        = VoxelSettings.IsoValue;
                float planetRadius  = VoxelSettings.PlanetRadius;
                float resolution    = VoxelSettings.Resolution;
                float octreeSize    = VoxelSettings.OctreeSize;


                ImGui::Begin("Voxel Settings");
                ImGui::Spacing();
                if (ImGui::DragFloat("IsoValue", &isoVal, 0.02f, -1.0f, 1.0f))
                    Regen = true;

                ImGui::Spacing();
                if (ImGui::DragFloat("Resolution", &resolution, 0.1f, 2.0f, 64.0f))
                    Regen = true;

                ImGui::Spacing();
                if (ImGui::DragFloat("Octree Size", &octreeSize, 0.1f, 4.0f, 64.0f))
                    Regen = true;

                ImGui::Spacing();
                ImGui::Checkbox("Wireframe", &wireframe);


                ImGui::Spacing();
                ImGui::Separator();

                VoxelSettings.IsoValue = isoVal;
                VoxelSettings.PlanetRadius = planetRadius;
                VoxelSettings.Resolution = resolution;
                VoxelSettings.OctreeSize = octreeSize;
                ImGui::End();
            }

            //PROFILING - The profile window
            {
                ImGui::Begin("Marching Cubes");
                ImGui::Text("Settings:");
                auto stats = Renderer3D::GetProfileData();
                ImGui::Text("Vert Count : %d", stats.MCPolyCount);
                ImGui::Text("Tri  Count : %d", stats.MCTriCount);
                ImGui::End();
            }
            {
                ImGui::Begin("Dual Contouring");
                ImGui::Text("Settings:");
                auto stats = Renderer3D::GetProfileData();
                ImGui::Text("Vert Count : %d", stats.DCPolyCount);
                ImGui::Text("Tri  Count : %d", stats.DCTriCount);
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

                const auto frameBuffer = RenderInstruction::GetApiPtr()->GetFrameBuffer();

                ImGui::Image((ImTextureID)frameBuffer->GetFrameBuffer(), ImVec2(ViewportSize.x, ViewportSize.y), {0,1}, {0,1});


                ImGui::End();
                ImGui::PopStyleVar();
            }

            //END DOCKSPACE
            ImGui::End();
        }

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
