#pragma once
#include <Framework/Scene/WorldSettings.h>
#include <Framework/Engine.h>

#include <Framework/Core/Time/DeltaTime.h>
#include <Framework/Core/Events/MouseEvent.h>
#include <Framework/Core/Events/AppEvents.h>
#include <Framework/Core/Events/KeyEvent.h>

#include "IsoSurface/DensityTextureGenerator.h"
#include "IsoSurface/MarchingCubes.h"
#include "IsoSurface/DualContouring.h"
#include "IsoSurface/MarchingCubesHP.h"
//#include "IsoSurface/DualContouringSPO.h"

#include "Panels/SceneHierarchyPanel.h"


#include <pix3.h>


namespace Engine
{


	class EditorLayer : public Layer
	{
	public:
		HMODULE GpuCaptureModule;
		HMODULE GpuTimingModule;
		PIXCaptureParameters PIXCaptureParams;
	public:

		EditorLayer();
		virtual ~EditorLayer() override;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(const DeltaTime& timer) override;
		void OnRender(const DeltaTime& timer) override;
		void OnImGuiRender() override;
		void OnEvent(Event& event) override;

	private:

		bool OnWindowResize(WindowResizeEvent& wndResize);

		bool OnMouseDown(MouseButtonPressedEvent& mEvent);
		bool OnMouseUp(MouseButtonReleasedEvent& mEvent);
		bool OnMouseMove(MouseMovedEvent& mEvent);

		void ConvertMouseCoordinates(float deltaTime);
	private:
		WorldSettings Settings;
		VoxelWorldSettings VoxelSettings;
		PerlinNoiseSettings PerlinSettings;
		CSGOperationSettings CsgOperationSettings;

		ScopePointer<DensityTextureGenerator> PerlinCompute;

		ScopePointer<MarchingCubes> MarchingCubes;
		ScopePointer<DualContouring> DualContouring;
		ScopePointer<MarchingCubesHP> MarchingCubesHP;
		//ScopePointer<DualContouringSPO> DualContourSPO;

		bool UpdateVoxels = false;
		bool UpdateTexture = true;

		bool Smooth = false;

	private:
		Editor::SceneHierarchyPanel SceneHierarchy;

		void Remap(float& x, float clx, float cmx, float nlx, float nmx);

		bool IsViewportFocused;
		bool IsViewportHovered;

		DirectX::XMFLOAT2 ViewportSize;

		float CurrentMouseX = 0;
		float CurrentMouseY = 0;
		float MouseDownX = 0;
		float MouseDownY = 0;



		bool LeftMButton = false;
		bool RightMButton = false;
		bool MouseMoved = false;

		AppTimeManager* TimerManager;
		RefPointer<Scene> Scene;
	};

}


