#pragma once
#include "Core/Core.h"
#include "Core/Window.h"
#include "Core/TimeStep.h"
#include "Core/Layer/LayerStack.h"
#include "Core/Events/AppEvents.h"

namespace DX12Framework
{
	class Application
	{
	protected:
		Application(const std::wstring& appName);

	public:
		virtual ~Application() = default;

		void Run();
		//void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		//inline Window* GetWindow() { return *Window; }

		inline static Application* Get() { return SingletonInstance; }

		//inline ImGuiLayer() { return ImGuiLayer; }


	private:

		// container for all the apps layers
		LayerStack LayerStack;

		// unique ptr to the system window
		RefPointer<Window> Window;

		//
		bool IsRunning;

		bool IsMinimised;

		float PreviousFrameTime;

	private:

		static Application* SingletonInstance;

	};

	// This must be defined on client.
	Application* CreateApplication();
}

