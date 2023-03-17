#pragma once
#include <Framework/Engine.h>
#include "Framework/Core/Time/DeltaTime.h"
#include "Framework/Core/Events/MouseEvent.h"
#include "Framework/Core/Events/AppEvents.h"
#include "Framework/Core/Events/KeyEvent.h"

#include "IsoSurface/PerlinCompute.h"
#include "IsoSurface/MarchingCubes.h"
#include "IsoSurface/DualContouring.h"
//#include "IsoSurface/MarchingCubesHP.h"
//#include "IsoSurface/DualContouringSPO.h"

namespace Engine
{

	class Texture;

	class EditorLayer : public Layer
	{
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


	private:
		VoxelWorldSettings VoxelSettings;
		PerlinNoiseSettings PerlinSettings;
		bool Regen = true;

		ScopePointer<PerlinCompute> PerlinCompute;

		ScopePointer<MarchingCubes> MarchingCubes;
		ScopePointer<DualContouring> DualContouring;
		//ScopePointer<DualContouringSPO> DualContourSPO;

		ScopePointer<Texture> ViewportTexture;

	private:
		void Remap(float& x, float clx, float cmx, float nlx, float nmx);

		bool IsViewportFocused;
		bool IsViewportHovered;

		DirectX::XMFLOAT2 ViewportSize;

		float CurrentMouseX = 0;
		float CurrentMouseY = 0;
		float MouseDownX = 0;
		float MouseDownY = 0;
		float MouseLastX = 0;
		float MouseLastY = 0;


		bool LeftMButton = false;
		bool RightMButton = false;
		bool MouseMoved = false;

		AppTimeManager* TimerManager;
		Scene* World;
	};

}


