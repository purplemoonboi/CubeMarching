#pragma once
#include "Framework/Core/Window.h"
#include "Framework/Core/Time/AppTimeManager.h"
#include "Framework/Core/Layer/LayerStack.h"
#include "Framework/Core/Events/AppEvents.h"
#include "Framework/ImGui/Layer/ImGuiLayer.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

#define WIN32_APP
#ifdef WIN32_APP
#include "Platform/Windows/WindowsWindow.h"
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
		explicit Application(HINSTANCE hInstance, const std::wstring& appName);
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
		static Application* Get() { return p_App; }

		AppTimeManager* GetApplicationTimeManager() { return &m_AppTimer; }

		Window* GetWindow() { return &m_Window; }

		[[nodiscard]] ImGuiLayer* GetImGuiLayer() const { return ImGuiLayer; }

	private:
		void UpdateTimer();

		// Contains all the application layers.
		LayerStack LayerStack;

		// Pointer to the imgui layer.
		ImGuiLayer* ImGuiLayer;

		bool IsRunning;

		float PreviousFrameTime;

	protected:
		static Application* p_App;

		//Application instance handle
		HINSTANCE p_AppInstance;

		//System window
		WindowsWindow m_Window;

		//A time manager
		AppTimeManager m_AppTimer;

		// Records core event updates.
		EventData m_EventBlob;
	};

	Application* CreateApplication(HINSTANCE hInstance, const std::wstring& appName);
}

