#pragma once
#include "Framework/Core/Window.h"
#include "Framework/Core/Time/AppTimeManager.h"
#include "Framework/Core/Layer/LayerStack.h"
#include "Framework/Core/Events/AppEvents.h"
#include "Framework/ImGui/Layer/ImGuiLayer.h"

#define WIN32_APP
#ifdef WIN32_APP
#include "Platform/Windows/Win32Window.h"
#endif

namespace Foundation
{

	struct EventData
	{
		INT32 X, Y;
		UINT64 Button;
		UINT8 MouseClicked = 0;
		UINT8 MouseReleased = 0;
		UINT8 MouseMoved = 0;
		std::function<void(Event&)> Callback;
	};

	class Application
	{
	protected:
		Application(HINSTANCE hInstance, const std::wstring& appName);
		DISABLE_COPY_AND_MOVE(Application);


	public:
		virtual ~Application();

		void Run();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		void OnApplicationEvent(Event& event);
		bool OnWindowResize(WindowResizeEvent& windowResize);

	public:
		/*..Win 32 callback..*/
		LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	public:
		/*..Getters..*/
		static Application* Get() { return pApp; }

		AppTimeManager* GetApplicationTimeManager() { return &AppTimer; }

		Win32Window* GetWindow() { return &Window; }

		[[nodiscard]] ImGuiLayer* GetImGuiLayer() const { return ImGuiLayer; }

	private:
		void UpdateTimer();

		// container for all the apps layers
		LayerStack LayerStack;

		ImGuiLayer* ImGuiLayer;

		bool IsRunning;
		float PreviousFrameTime;

	protected:
		static Application* pApp;

		//Application instance handle
		HINSTANCE AppInstance;

		//System window
		Win32Window Window;

		//A time manager
		AppTimeManager AppTimer;

		
		EventData EventBlob;
	};

	Application* CreateApplication(HINSTANCE hInstance, const std::wstring& appName);
}

