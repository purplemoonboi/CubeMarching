#pragma once
#include "Framework/Core/Core.h"
#include "Framework/Core/Window.h"
#include "Framework/Core/Time/DeltaTime.h"
#include "Framework/Core/Time/AppTimeManager.h"
#include "Framework/Core/Layer/LayerStack.h"
#include "Framework/Core/Events/AppEvents.h"
#include "Platform/Windows/WindowsWindow.h"

namespace DX12Framework
{

	class DX12GraphicsContext;


	class Application
	{
	

	protected:
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(HINSTANCE hInstance, const std::wstring& appName);

	public:
		virtual ~Application() = default;

		void OnApplicatonEvent(Event& event);

		void Run();
		//void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		// @brief Returns a pointer to the active window
		WindowsWindow* GetWindow() { return Window; }

		// @brief Returns this application
		static Application* Get() { return SingletonInstance; }

		//inline ImGuiLayer() { return ImGuiLayer; }

		// @brief Captures a window resize event, if true dispatch a
		//		  invalidate buffer command. (Rebuilds buffer)
		bool OnWindowResize(WindowResizeEvent& windowResize);

	private:

		void UpdateTimer();

		// container for all the apps layers
		LayerStack LayerStack;

		// Reflects the state of the app currently running
		bool IsRunning;

		float PreviousFrameTime;

	protected:

		static Application* SingletonInstance;

		// unique ptr to the system window
		WindowsWindow* Window;

		//A time manager
		AppTimeManager Timer;

		//Application instance handle
		HINSTANCE AppInstance;

	};

	// This must be defined on client.
	Application* CreateApplication(HINSTANCE hInstance, const std::wstring& appName);
}

