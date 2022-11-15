#pragma once
#include "Framework/Core/Window.h"
#include "Framework/Core/Time/AppTimeManager.h"
#include "Framework/Core/Layer/LayerStack.h"
#include "Framework/Core/Events/AppEvents.h"
#include "Platform/Windows/WindowsWindow.h"

namespace Engine
{

	class DX12GraphicsContext;


	class Application
	{
	

	protected:
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(HINSTANCE hInstance, const std::wstring& appName);
	public:
		virtual ~Application();

		// @brief Processes application based events 
		void OnApplicatonEvent(Event& event);

		// @brief Core application loop
		void Run();

		// @brief Add a layer to the layer stack
		void PushLayer(Layer* layer);

		// @brief Add an overlay to the stack
		// @note Primarily for third party software
		//		 and plugins
		void PushOverlay(Layer* overlay);

		// @brief Returns a pointer to the active window
		WindowsWindow& GetWindow() { return Window; }

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

		//Singleton pointer to this application
		static Application* SingletonInstance;

		//Application instance handle
		HINSTANCE AppInstance;

	protected:

		// unique ptr to the system window
		WindowsWindow Window;

		//A time manager
		AppTimeManager AppTimer;

	};

	// This must be defined on client.
	Application* CreateApplication(HINSTANCE hInstance, const std::wstring& appName);
}

