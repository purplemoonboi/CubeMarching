#include "EditorLayer.h"

#include "Framework/Scene/Scene.h"
#include "Framework/Core/Input/Input.h"

#include <../ImGui/imgui.h>
#include <../ImGui/imgui_internal.h>

#include "Framework/Core/Compute/ComputeInstruction.h"
#include "Framework/Renderer/Textures/RenderTarget.h"
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
        Scene = CreateRef<class Scene>("Test Scene");
        TimerManager = Application::Get()->GetApplicationTimeManager();

        PerlinCompute = CreateScope<class DensityTextureGenerator>();

        MarchingCubes = CreateScope<class MarchingCubes>();
        MarchingCubesHP = CreateScope<class MarchingCubesHP>();

        DualContouring = CreateScope<class DualContouring>();
        RadixSort = CreateScope<class Radix>();
    	//DualContourSPO   = CreateScope < class DualContouringSPO >();

    }

    EditorLayer::~EditorLayer()
    {

        MarchingCubes.reset();
        MarchingCubes = nullptr;

        DualContouring.reset();
        DualContouring = nullptr;

        PerlinCompute.reset();
        PerlinCompute = nullptr;

    }

    void EditorLayer::OnAttach()
    {
#ifdef USE_PIX

        GpuCaptureModule = PIXLoadLatestWinPixGpuCapturerLibrary();
        GpuTimingModule = PIXLoadLatestWinPixTimingCapturerLibrary();

        PIXCaptureParams.GpuCaptureParameters.FileName = L"assets/pix/MarchingCubes.wpix";
        /*PIXCaptureParams.TimingCaptureParameters.FileName = L"assets/pix/MarchingCubes.wpix";
        PIXCaptureParams.TimingCaptureParameters.CaptureGpuTiming = TRUE;
        PIXCaptureParams.TimingCaptureParameters.CaptureHeapAllocEvents = TRUE;
        PIXCaptureParams.TimingCaptureParameters.CaptureCallstacks = TRUE;
        PIXCaptureParams.TimingCaptureParameters.CapturePixMemEvents = TRUE;*/

#endif

        const auto api = RenderInstruction::GetApiPtr();

        ComputeInstruction::Init(api->GetGraphicsContext());
        const auto csApi = ComputeInstruction::GetComputeApi();

        RenderInstruction::ResetGraphicsCommandList();

        PerlinCompute->Init(csApi, api->GetMemoryManager());
        PerlinCompute->PerlinFBM(PerlinSettings, CsgOperationSettings);

    	MarchingCubes->Init(csApi, api->GetMemoryManager());
    	Renderer3D::CreateVoxelTerrain(MarchingCubes->GetVertices(),
				MarchingCubes->GetIndices(), "MarchingTerrain", Transform(0, 0, 0));

        /* polygonise the texture with marching cubes */

    	
    	/*DualContouring->Init(csApi, api->GetMemoryManager());
           Renderer3D::CreateVoxelTerrain(DualContouring->GetVertices(), 
               DualContouring->GetIndices(), "DualTerrain", Transform(0, 0, 0));*/
               
       /* RadixSort->Init(csApi, api->GetMemoryManager());
        RadixSort->SortChunk(VoxelSettings);*/

    	//MarchingCubesHP->Init(csApi, api->GetMemoryManager());

        //DualContourSPO->Init(csApi, api->GetMemoryManager());

        Renderer3D::CreateMesh("Brush", Transform(0, 0, 0, 0,0,0), 1);

        RenderInstruction::ExecGraphicsCommandList();


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


            

            // Update Texture
            if (RightMButton)
            {
                CsgOperationSettings.IsMouseDown = 1;
                ConvertMouseCoordinates(deltaTime);
                UpdateTexture = true;
            }
            else
            {
                CsgOperationSettings.IsMouseDown = 0;
            }
     

        }



        Scene->OnUpdate(deltaTime.GetSeconds(), TimerManager->TimeElapsed());

        if (UpdateTexture)
        {
            PerlinSettings.ChunkCoord = { (float)0, 0, (float)0 };
            PerlinCompute->PerlinFBM(PerlinSettings, CsgOperationSettings);
            UpdateTexture = false;
            UpdateVoxels = true;
        }

        if(UpdateVoxels)
        {
            
            /* polygonise the texture with marching cubes */
           /* HRESULT hr = PIXBeginCapture(PIX_CAPTURE_GPU, &PIXCaptureParams);
            if (!SUCCEEDED(hr))
            {
                CORE_ERROR("GPU capture failed...")
            }
            else
            {
                CORE_TRACE("Beginning GPU capture...");
            }*/

           
            switch (algo)
            {
            case 0:
                MarchingCubes->Dispatch(VoxelSettings, PerlinCompute->GetTexture());
                Renderer3D::SetBuffer("MarchingTerrain", MarchingCubes->GetVertices(), MarchingCubes->GetIndices());
                break;
            case 1:
                /* polygonise the texture with surface nets*/

                break;
            case 2:
                /* polygonise the texture with dual contouring */
                /*
                DualContouring->Dispatch(VoxelSettings, PerlinCompute->GetTexture());
                Renderer3D::SetBuffer("DualTerrain", DualContouring->GetVertices(), DualContouring->GetIndices());
                */
                break;
            case 3:
                /* polygonise the texture with improved marching cubes */
                /*
                MarchingCubesHP->Polygonise(VoxelSettings, PerlinCompute->GetTexture());
                Renderer3D::SetBuffer("MarchingTerrain", MarchingCubesHP->GetVertexBuffer(), MarchingCubesHP->GetIndexBuffer());
                */
                break;

            }

            /*if(SUCCEEDED(hr))
            {
                hr = PIXEndCapture(FALSE);
				if(!SUCCEEDED(hr))
				{
                    CORE_ERROR("Could not end GPU capture successfully...");
				}
                else
                {
                    CORE_TRACE("Ending GPU capture...");

                }
            }*/

            UpdateVoxels = false;

        }

        auto rt = RenderInstruction::GetApiPtr()->GetRenderTextureAlbedo();

        if(ViewportSize.x > 0 && ViewportSize.x < 8196 && ViewportSize.y > 0 && ViewportSize.y < 8196)
        {
            if (rt->GetWidth() != (INT32)ViewportSize.x || rt->GetHeight() != (INT32)ViewportSize.y)
            {
                mc->SetAspectRatio(ViewportSize.x, ViewportSize.y);
                mc->RecalculateAspectRatio(ViewportSize.x, ViewportSize.y);
                rt->OnResize((INT32)ViewportSize.x, (INT32)ViewportSize.y);
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
        {
            // If we removed all the options we are showcasing, this demo would become:
            //     void ShowExampleAppDockSpace()
            //     {
            //         ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
            //     }
           

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
                    UpdateTexture = true;

                if (ImGui::DragFloat("Frequency", &freq, 0.1f, 0.001f, 0.999f))
                    UpdateTexture = true;

                ImGui::Spacing();
                if (ImGui::DragFloat("Gain", &gain, 0.1f, 0.1f, 4.0f))
                    UpdateTexture = true;

                ImGui::Spacing();
                if (ImGui::DragFloat("Ground Height", &groundHeight, 0.1f, 2.0f, (float)ChunkHeight))
                    UpdateTexture = true;


                ImGui::Separator();
                ImGui::End();
                PerlinSettings.Frequency = freq;
                PerlinSettings.Gain = gain;
                PerlinSettings.GroundHeight = groundHeight;
                PerlinSettings.Octaves = octaves;

                float isoVal         = VoxelSettings.IsoValue;
                bool useBinarySearch = VoxelSettings.UseBinarySearch;
                INT32 resolution = VoxelSettings.Resolution;
                bool useTexture = VoxelSettings.UseTexture;


                ImGui::Begin("Voxel Settings");
                ImGui::Spacing();
                if (ImGui::DragFloat("IsoValue", &isoVal, 0.02f, -1.0f, 1.0f))
                    UpdateVoxels = true;

                ImGui::Spacing();
                if (ImGui::DragInt("Resolution", &resolution, 0.1f, 2.0f, 64.0f))
                    UpdateVoxels = true;

                ImGui::Spacing();
                if (ImGui::Checkbox("UseBinarySearch", &useBinarySearch))
                    UpdateVoxels = true;

                ImGui::Spacing();
                if (ImGui::Checkbox("UseTexture", &useTexture))
                    UpdateVoxels = true;

                ImGui::Spacing();
                ImGui::Checkbox("Wireframe", &wireframe);


                ImGui::Spacing();
                ImGui::Separator();

                VoxelSettings.IsoValue = isoVal;
                VoxelSettings.UseBinarySearch = useBinarySearch;
                VoxelSettings.Resolution = resolution;
                VoxelSettings.UseTexture = useTexture;
                ImGui::End();
            }

            // CSG SETTINGS
            {
                float radius = CsgOperationSettings.Radius;
                float rayDist = CsgOperationSettings.RayDistance;
                ImGui::Begin("CSG Settings");
                ImGui::Spacing();
                if (ImGui::DragFloat("Brush Radius", &radius, 0.02f, 1.0f, 10.0f))
                {
                    CsgOperationSettings.Radius = radius;
                }

                ImGui::Spacing();
                if (ImGui::DragFloat("Ray Range", &rayDist, 0.02f, 2.0f, 100.0f))
                {
                    CsgOperationSettings.RayDistance = rayDist;
                }


                ImGui::Spacing();
                const char* primitives[] = { "Box", "Sphere", "Cylinder", "Torus" };
                static const char* currentPrim = nullptr;

                if (ImGui::BeginCombo("Density Primitive", currentPrim)) // The second parameter is the label previewed before opening the combo.
                {
                    for (int n = 0; n < IM_ARRAYSIZE(primitives); ++n)
                    {
                        const bool isSelected = (currentPrim == primitives[n]); // You can store your selection however you want, outside or inside your objects
                        if (ImGui::Selectable(primitives[n], isSelected))
                        {
                            currentPrim = primitives[n];
                            CsgOperationSettings.DensityPrimitive = n;
                        }
                        if (isSelected)
                        {
                            ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::Spacing();
                const char* operations[] = { "Add", "Subtract", "Union" };
                static const char* currentOperation = nullptr;

                if (ImGui::BeginCombo("Csg Operation", currentOperation)) // The second parameter is the label previewed before opening the combo.
                {
                    for (int n = 0; n < IM_ARRAYSIZE(operations); ++n)
                    {
                        const bool isSelected = (currentOperation == operations[n]); // You can store your selection however you want, outside or inside your objects
                        if (ImGui::Selectable(operations[n], isSelected))
                        {
                            currentOperation = operations[n];
                            CsgOperationSettings.CsgOperation = n;
                        }
                        if (isSelected)
                        {
                            ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::End();
            }

            ImGui::Spacing();
            const char* algos[] = { "Marching Cubes", "Surface Nets", "Dual Contouring" };
            static const char* currentAlgo = nullptr;

            if (ImGui::BeginCombo("Isosurface Algorithms", currentAlgo)) // The second parameter is the label previewed before opening the combo.
            {
                for (int n = 0; n < IM_ARRAYSIZE(algos); ++n)
                {
                    const bool isSelected = (currentAlgo == algos[n]); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(algos[n], isSelected))
                    {
                        currentAlgo = algos[n];
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                    }
                }
                ImGui::EndCombo();
            }
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

                
                auto rt = RenderInstruction::GetApiPtr()->GetRenderTextureAlbedo();
                ImGui::Image((ImTextureID)rt->GetTexture(), ImVec2(ViewportSize.x, ViewportSize.y), {0,0}, {1,1});


                ImGui::End();
                ImGui::PopStyleVar();
            }



            SceneHierarchy.OnImGuiRender();

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

    void EditorLayer::ConvertMouseCoordinates(float deltaTime)
    {
        float xx = CurrentMouseX / ViewportSize.x;
        float yy = CurrentMouseY / ViewportSize.y;

        const float x = (2.0f * xx) - 1.0f;
        const float y = 1.0f - (2.0f * yy);

        const auto* camera = Scene->GetSceneCamera();

        XMFLOAT4X4 inverseProjection;
        XMStoreFloat4x4(&inverseProjection, XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera->GetProjection())));

        const float vx = x / inverseProjection(0, 0);
        const float vy = y / inverseProjection(1, 1);
        const float vz = 1.0f;

        XMFLOAT4X4 inverseView;
        XMStoreFloat4x4(&inverseView, XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera->GetView())));

        XMFLOAT3 camPos = camera->GetPosition();
        /*Remap(camPos.x, -1024.0f, 1024.0f, 0.0f, (float) VoxelSettings.Resolution);
        Remap(camPos.y, -1024.0f, 1024.0f, 0.0f, (float)VoxelSettings.Resolution);
        Remap(camPos.z, -1024.0f, 1024.0f, 0.0f, (float)VoxelSettings.Resolution);*/

        XMFLOAT3 rayOrigin = camPos;
        XMFLOAT3 rayDirection(
            vx * inverseView(0, 0) + vy * inverseView(1, 0) + vz * inverseView(2, 0),
            vx * inverseView(0, 1) + vy * inverseView(1, 1) + vz * inverseView(2, 1),
            vx * inverseView(0, 2) + vy * inverseView(1, 2) + vz * inverseView(2, 2));

        // Normalize the direction vector to get a unit vector
        rayDirection.x *= CsgOperationSettings.RayDistance;
        rayDirection.y *= CsgOperationSettings.RayDistance;
        rayDirection.z *= CsgOperationSettings.RayDistance;

        CXMVECTOR direction = XMLoadFloat3(&rayDirection);
        XMStoreFloat3(&rayDirection, direction);

        XMFLOAT3 camForward = camera->GetForward();

        float xf = rayOrigin.x + camForward.x + rayDirection.x;
        float yf = rayOrigin.y + camForward.y + rayDirection.y;
        float zf = rayOrigin.z + camForward.z + rayDirection.z;


        CsgOperationSettings.MousePos = { xf, yf, zf };
        CORE_TRACE("Mouse Pos W: {0}, {1}, {2}", xf, yf, zf);
        CORE_TRACE("Camera Pos W: {0}, {1}, {2}", rayOrigin.x, rayOrigin.y, rayOrigin.z);

        D3D12RenderItem* brush = dynamic_cast<D3D12RenderItem*>(Renderer3D::GetRenderItem(2));

        float rad = CsgOperationSettings.Radius;
        auto transform = XMMatrixTranslation(xf, yf, zf);
        XMStoreFloat4x4(&brush->World, transform);
        brush->NumFramesDirty++;
    }


    void EditorLayer::Remap(float& x, float clx, float cmx, float nlx, float nmx)
    {
        x = nlx + (x - clx) * (nmx - nlx) / (cmx - clx);
    }
}
