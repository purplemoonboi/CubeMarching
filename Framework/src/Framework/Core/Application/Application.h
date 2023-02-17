#pragma once
#include "Framework/Core/Window.h"
#include "Framework/Core/Time/AppTimeManager.h"
#include "Framework/Core/Layer/LayerStack.h"
#include "Framework/Core/Events/AppEvents.h"
#include "Framework/Core/Events/MouseEvent.h"
#include "Platform/Windows/Win32Window.h"

#include "Framework/ImGui/Layer/ImGuiLayer.h"


namespace Engine
{

	class D3D12Context;


	class Application
	{
	

	protected:
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(HINSTANCE hInstance, const std::wstring& appName);
	public:
		virtual ~Application();

		// @brief Processes application based events 
		void OnApplicationEvent(Event& event);

		// @brief Core application loop
		void Run();

		// @brief Add a layer to the layer stack
		void PushLayer(Layer* layer);

		// @brief Add an overlay to the stack
		// @note Primarily for third party software
		//		 and plugins
		void PushOverlay(Layer* overlay);

		// @brief Returns a pointer to the active window
		Win32Window& GetWindow() { return Window; }

		// @brief Returns a reference to the app time manager
		AppTimeManager* GetApplicationTimeManager() { return &AppTimer; }

		ImGuiLayer* GetImGuiLayer() { return ImGuiLayer; }

		// @brief Returns this application
		static Application* Get() { return SingletonInstance; }

		//inline ImGuiLayer() { return ImGuiLayer; }


		// @brief Captures a window resize event, if true dispatch a
		//		  invalidate buffer command. (Rebuilds buffer)
		bool OnWindowResize(WindowResizeEvent& windowResize);

		// @brief Event callback bound to the system, processes and handles
		//		  received events from the user.
		// @param[in] A handle to the window
		// @param[in] The event received from the system
		// @param[in] A wide parameter
		// @param[in] A long parameter
		LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:

		void UpdateTimer();

		// container for all the apps layers
		LayerStack LayerStack;

		ImGuiLayer* ImGuiLayer;

		// Reflects the state of the app currently running
		bool IsRunning;

		float PreviousFrameTime;

	protected:

		//Singleton pointer to this application
		static Application* SingletonInstance;

		//Application instance handle
		HINSTANCE AppInstance;

	protected:

		// System window
		Win32Window Window;

		ScopePointer<GraphicsContext> GraphicsContext;

		// System Keyboard
		//KeyBoard Keyboard;

		//A time manager
		AppTimeManager AppTimer;

		/* Used to capture mouse input from the OS */
		struct MouseInputEventData
		{
			INT32 X, Y;
			UINT64 Button;
			UINT8 MouseClicked = 0;
			UINT8 MouseReleased = 0;
			UINT8 MouseMoved = 0;
			std::function<void(Event&)> CallBack;
		};

		MouseInputEventData MouseData;

	};

	// This must be defined on client.
	Application* CreateApplication(HINSTANCE hInstance, const std::wstring& appName);
}

