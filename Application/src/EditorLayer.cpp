#include "EditorLayer.h"

#include "Framework/Scene/Scene.h"

#include <../ImGui/imgui.h>
#include <../ImGui/imgui_internal.h>

#include "Framework/Core/Compute/ComputeInstruction.h"
#include "Framework/Renderer/Textures/Texture.h"
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

        MarchingCubes = CreateScope<class MarchingCubes>();
        PerlinCompute = CreateScope<class PerlinCompute>();
        DualContour   = CreateScope < class DualContouring >();
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

        ShaderArgs args =
        { L"assets\\shaders\\MarchingCube.hlsl" ,
            "GenerateChunk",
            "cs_5_0"
        };
        MarchingCubes->Init(csApi, api->GetMemoryManager(), args);

        //DualContour->Init(csApi, api->GetMemoryManager());

        ShaderArgs perlinArgs = 
        {
            L"assets\\shaders\\Perlin.hlsl",
            "ComputeNoise3D",
            "cs_5_0"
        };
        PerlinCompute->Init(csApi, api->GetMemoryManager(), perlinArgs);

        ViewportTexture = Texture::Create(0, 1920U, 1080U, TextureFormat::RGBA_UINT_UNORM);

        RenderInstruction::ExecGraphicsCommandList();
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


        World->OnUpdate(deltaTime.GetSeconds(), TimerManager->TimeElapsed());


		
        static INT32 offsetx = 0;
        static INT32 offsetz = 0;
        static UINT32 count = 0;
    
        if (Regen)
        {
            Regen = false;
            PerlinSettings.ChunkCoord = { (float)offsetx, 0, (float)offsetz }; 
            PerlinCompute->Dispatch(PerlinSettings, ChunkWidth, ChunkHeight, ChunkWidth);
            MarchingCubes->Dispatch(VoxelSettings, { (float)offsetx, 0, (float)offsetz }, PerlinCompute->GetTexture(), ChunkWidth, ChunkHeight, ChunkWidth);

            // @note thread count varies per pass.
            // TODO: Remove thread count from args
            //DualContour->Dispatch(VoxelSettings, nullptr, 0, 0, 0);

            if (MarchingCubes->GetTerrainMesh() != nullptr)
            {
                float halfxz = static_cast<float>(ChunkWidth) * 0.5f;
                Renderer3D::CreateCustomMesh(std::move(MarchingCubes->GetTerrainMesh()), "Terrain", Transform(-halfxz, 0, -halfxz));
            }
        }
    }

    void EditorLayer::OnRender(const DeltaTime& timer)
    {
        World->OnRender(timer.GetSeconds(), TimerManager->TimeElapsed());
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

           // //..................................DOCKSPACE..................................//


            ImGuiIO& io = ImGui::GetIO();
            ImGuiStyle& style = ImGui::GetStyle();
            const float minWindowSize = style.WindowMinSize.x;
            style.WindowMinSize.x = 360.0f;
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }

           // style.WindowMinSize.x = minWindowSize;

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

            //VOXEL SETTINGS 
            {
                ImGui::Begin("Noise Settings");
                ImGui::Separator();
                ImGui::Spacing();
                float freq = PerlinSettings.Frequency;
                float gain = PerlinSettings.Gain;
                float groundHeight = PerlinSettings.GroundHeight;
                INT32 octaves = PerlinSettings.Octaves;
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
                if (ImGui::DragFloat("IsoValue", &isoVal, 0.1f, -0.001f, 0.999f))
                    Regen = true;

                ImGui::Spacing();
                if (ImGui::DragFloat("Planet Radius", &planetRadius, 0.1f, 2.0f, 10.0f))
                    Regen = true;

                ImGui::Spacing();
                if (ImGui::DragFloat("Resolution", &resolution, 0.1f, 2.0f, 64.0f))
                    Regen = true;

                ImGui::Spacing();
                if (ImGui::DragFloat("Octree Size", &octreeSize, 0.1f, 4.0f, 64.0f))
                    Regen = true;

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
                ImGui::Begin("Profiling");
                ImGui::Text("Settings:");
                auto stats = Renderer3D::GetProfileData();
               // ImGui::Text("Draw Calls : %d", stats.DrawCalls);
                ImGui::Text("Vert Count : %d", stats.PolyCount);
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

           /*     const auto frameBuffer = RenderInstruction::GetApiPtr()->GetFrameBuffer();
                ViewportTexture->Copy(frameBuffer->GetFrameBuffer());*/

                ImGui::Image((ImTextureID)ViewportTexture->GetTexture(), ImVec2(ViewportSize.x, ViewportSize.y), {0,1}, {1,0});


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
