#pragma once
#include "Framework/Core/Layer/Layer.h"
#include "Framework/Core/Events/AppEvents.h"
#include "Framework/Core/Events/MouseEvent.h"
#include "Framework/Core/Events/KeyEvent.h"


namespace Foundation
{
	enum class ImGuiRenderingApi
	{
		DX11 = 0,
		DX12
	};

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		ImGuiLayer(ImGuiRenderingApi api);
		~ImGuiLayer() override;

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event& event) override;
		void OnUpdate(const DeltaTime& timer) override {}
		void OnRender(const DeltaTime& timer) override {}
		void OnImGuiRender() override;
		static void Begin();
		static void End();

		void SetBlockEvents(bool block) { BlockEvents = block; }
	
		void SetDarkThemeColours();

	private:
		bool BlockEvents;
		float Time;

		ImGuiRenderingApi ImGuiRenderingAPI;
	};
}
