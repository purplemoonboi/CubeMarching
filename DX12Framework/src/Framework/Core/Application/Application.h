#pragma once
#include "Framework/Core/Core.h"
#include "Framework/Core/Window.h"
#include "Framework/Core/Time/DeltaTime.h"
#include "Framework/Core/Time/AppTimeManager.h"
#include "Framework/Core/Layer/LayerStack.h"
#include "Framework/Core/Events/AppEvents.h"

#include "Platform/DirectX12/WindowsWindow.h"

namespace DX12Framework
{
	class Application
	{
	

	protected:
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(HINSTANCE hInstance, const std::wstring& appName);

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
		WindowsWindow* Window;

		//A time manager
		AppTimeManager Timer;

		// Reflects the state of the app currently running
		bool IsRunning;

		float PreviousFrameTime;


	protected:

		static Application* SingletonInstance;

		//Application instance handle
		HINSTANCE AppInstance;

	private:


	};

	// This must be defined on client.
	Application* CreateApplication(HINSTANCE hInstance, const std::wstring& appName);
}

