#include "Framework/cmpch.h"
#include "Application.h"

#include "Framework/Core/Log/Log.h"
#include "Framework/Renderer/Renderer3D/Renderer.h"

#include <imgui.h>


// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Foundation
{
	Application* Application::p_App = nullptr;

	
	LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return Application::Get()->MsgProc(hwnd, msg, wParam, lParam);
	}

	Application::Application(HINSTANCE hInstance, const std::wstring& appName)
		:
			IsRunning(true)
		,	ImGuiLayer(nullptr)
		,	PreviousFrameTime(0.0f)
	{
		Log::Init();

		//Check if an app instance exists
		CORE_ASSERT(!p_App, "An application instance already exists!");
		p_App = this;

		// Create a windows window.
		WindowProperties winProps = {};
		winProps.WinInstance = hInstance;
		winProps.WinHandle = 0;

		m_Window = WindowsWindow(winProps);

		// Bind the applications on event function to capture application specific events.
		m_Window.SetEventCallBack(BIND_DELEGATE(Application::OnApplicationEvent));

		m_EventBlob.Callback = BIND_DELEGATE(Application::OnApplicationEvent);

		auto handle = static_cast<HWND>(m_Window.GetNativeWindow());
		Graphics::Renderer::Init(&m_Window);

		ImGuiLayer = new class ImGuiLayer();
		PushOverlay(ImGuiLayer);
		IsRunning = true;
	}

	Application::~Application()
	{
		
	}

	void Application::OnApplicationEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<WindowResizeEvent>(BIND_DELEGATE(Application::OnWindowResize));

		for(auto itr = LayerStack.end(); itr != LayerStack.begin();)
		{
			(*--itr)->OnEvent(event);
			if(event.HasEventBeenHandled())
			{
				break;
			}
		}
	}

	void Application::Run()
	{
		MSG message = { 0 };

		m_AppTimer.Reset();

		while(message.message != WM_QUIT)
		{
			if(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			else
			{
				m_AppTimer.Tick();

				//Process any events...
				m_Window.OnUpdate();


				if (!m_Window.IsMinimised() && IsRunning)
				{

					if (m_EventBlob.MouseClicked > 0)
					{
						m_EventBlob.MouseClicked = 0;
						MouseButtonPressedEvent me(m_EventBlob.Button, m_EventBlob.X, m_EventBlob.Y);
						m_EventBlob.Callback(me);
					}
					if (m_EventBlob.MouseReleased > 0)
					{
						MouseButtonReleasedEvent me(m_EventBlob.Button, m_EventBlob.X, m_EventBlob.Y);
						m_EventBlob.Callback(me);
					}
					if (m_EventBlob.MouseMoved == 1)
					{
						m_EventBlob.MouseMoved = 0;
						MouseMovedEvent mm(m_EventBlob.X, m_EventBlob.Y, m_EventBlob.Button);
						m_EventBlob.Callback(mm);
					}


					UpdateTimer();

					//Update each layer
					for (Layer* layer : LayerStack)
					{
						layer->OnUpdate(&m_AppTimer);
					}

					//Render each layer
					for (Layer* layer : LayerStack)
					{
						layer->OnRender(&m_AppTimer);
					}

					ImGuiLayer::Begin();
					for(Layer* overlay : LayerStack)
					{
						overlay->OnImGuiRender();
					}
					ImGuiLayer::End();

				}
			}
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	bool Application::OnWindowResize(WindowResizeEvent& windowResize)
	{
		CORE_TRACE("Resize event");

		if(windowResize.GetHeight() == 0 || windowResize.GetWidth() == 0 || 
			windowResize.GetWidth() > 8196 || windowResize.GetHeight() > 8196)
		{
			return false;
		}

		Graphics::Renderer::OnWindowResize(0, 0, windowResize.GetWidth(), windowResize.GetHeight());

		return false;
	}


	LRESULT Application::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if(ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		{
			return true;
		}

		bool isClosing = false;
		bool isMaximised = false;
		bool isMinimised = false;
		bool isResizing = false;

		switch (msg)
		{
		case WM_QUIT:
			isClosing = true;
			return 0;
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				m_AppTimer.Stop();
			}
			else
			{
				m_AppTimer.Start();
			}
			return 0;
		case WM_SIZE:
			// Save the new client area dimensions.
			if (wParam == SIZE_MINIMIZED)
			{
				isMinimised = true;
				isMaximised = false;
				m_Window.UpdateWindowData(LOWORD(lParam), HIWORD(lParam), true, false, false, false, false);
				m_AppTimer.Stop();
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				isMinimised = false;
				isMaximised = true;
				m_Window.UpdateWindowData(LOWORD(lParam), HIWORD(lParam), false, true, false, false, false);
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (isMinimised)
				{
					isMinimised = false;
				}

				// Restoring from maximized state?
				else if (isMaximised)
				{
					isMaximised = false;
				}
				else if (isResizing)
				{
					/*don't resize here, wait until the WM_ENTERSIZEMOVE has fired. */
				}
				else // Api call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					m_Window.UpdateWindowData(LOWORD(lParam), HIWORD(lParam), false, false, false, false, false);
				}


			}
			return 0;
			// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
		case WM_ENTERSIZEMOVE:
			isResizing = true;
			return 0;
			// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
			// Here we reset everything based on the new window dimensions.
		case WM_EXITSIZEMOVE:
			isResizing = false;
			m_AppTimer .Start();
			return 0;
			// WM_DESTROY is sent when the window is being destroyed.
		case WM_DESTROY:
			Graphics::Renderer::Clean();
			PostQuitMessage(0);
			return 0;
			// The WM_MENUCHAR message is sent when a menu is active and the user presses 
			// a key that does not correspond to any mnemonic or accelerator key. 
		case WM_MENUCHAR:
			// Don't beep when we alt-enter.
			return MAKELRESULT(0, MNC_CLOSE);

			// Catch this message so to prevent the window from becoming too small.
		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			return 0;


		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			SetCapture(static_cast<HWND>(m_Window.GetNativeWindow()));
			m_EventBlob.MouseClicked = 1;
			m_EventBlob.MouseReleased = 0;
			m_EventBlob.X = GET_X_LPARAM(lParam);
			m_EventBlob.Y = GET_Y_LPARAM(lParam);
			m_EventBlob.Button = wParam;

				return 0;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			ReleaseCapture();

			m_EventBlob.MouseClicked = 0;
			m_EventBlob.MouseReleased = 1;
			m_EventBlob.X = GET_X_LPARAM(lParam);
			m_EventBlob.Y = GET_Y_LPARAM(lParam);
			m_EventBlob.Button = wParam;

				return 0;
		case WM_MOUSEMOVE:

			m_EventBlob.Button = wParam;
			m_EventBlob.X = GET_X_LPARAM(lParam);
			m_EventBlob.Y = GET_Y_LPARAM(lParam);
			m_EventBlob.MouseMoved = 1;

				return 0;
		case WM_KEYDOWN:

		

			return 0;
		case WM_KEYUP:

		

			if (wParam == VK_ESCAPE)
			{
				PostQuitMessage(0);
			}
		

				return 0;
		}


		return DefWindowProc(hwnd, msg, wParam, lParam);
	}


	void Application::UpdateTimer()
	{

		static int frameCnt = 0;
		static float timeElapsed = 0.0f;

		frameCnt++;

		// Compute averages over one second period.
		if ((m_AppTimer.TimeElapsed() - timeElapsed) >= 1.0f)
		{
			float fps = (float)frameCnt; // fps = frameCnt / 1
			float mspf = 1000.0f / fps;

			std::wstring fpsStr = std::to_wstring(fps);
			std::wstring mspfStr = std::to_wstring(mspf);

			std::wstring windowText = L"Foundation fps: " + fpsStr +
				L"   mspf: " + mspfStr;

			const auto wnd = static_cast<HWND>(m_Window.GetNativeWindow());
			SetWindowText(wnd, windowText.c_str());

			// Reset for next average.
			frameCnt = 0;
			timeElapsed += 1.0f;
		}

	}
}
