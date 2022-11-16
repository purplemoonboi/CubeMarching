#include "Framework/cmpch.h"
#include "Application.h"

#include "Framework/Core/Log/Log.h"
#include "Framework/Renderer/Renderer.h"


namespace Engine
{
	Application* Application::SingletonInstance = nullptr;

	Application::Application(HINSTANCE hInstance, const std::wstring& appName)
		:
		IsRunning(true)
	{
		Log::Init();

		//Check if an app instance exists
		CORE_ASSERT(!SingletonInstance, "An application instance already exists!");
		SingletonInstance = this;

		// TODO: Create a windows window
		Window = WindowsWindow(hInstance, 1920, 1080, L"Engine");
		// Bind the applications on event function to capture application specific events.
		Window.SetEventCallBack(BIND_DELEGATE(Application::OnApplicatonEvent));

		// TODO: Initialise the renderer
		Renderer::InitD3D(static_cast<HWND>(Window.GetNativeWindow()), 1920, 1080);
	}

	Application::~Application()
	{
	}

	void Application::OnApplicatonEvent(Event& event)
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

		AppTimer.Reset();

		while(message.message != WM_QUIT)
		{
			if(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			else
			{

				//Process any events...
				Window.OnUpdate();

				if (!Window.IsWndMinimised() && IsRunning)
				{
					AppTimer.Tick();

					UpdateTimer();

					//Update each layer
					for(Layer* layer : LayerStack)
					{
						layer->OnUpdate(AppTimer.DeltaTime());
					}

					//Render each layer
					for(Layer* layer : LayerStack)
					{
						layer->OnRender(AppTimer.DeltaTime());
					}
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

		if(windowResize.GetHeight() == 0 || windowResize.GetWidth() == 0)
		{
			IsRunning = false;
			return false;
		}

		IsRunning = true;
		Renderer::OnWindowResize(0, 0, windowResize.GetWidth(), windowResize.GetHeight());

		return false;
	}

	void Application::UpdateTimer()
	{

		static int frameCnt = 0;
		static float timeElapsed = 0.0f;

		frameCnt++;

		// Compute averages over one second period.
		if ((AppTimer.TimeElapsed() - timeElapsed) >= 1.0f)
		{
			float fps = (float)frameCnt; // fps = frameCnt / 1
			float mspf = 1000.0f / fps;

			std::wstring fpsStr = std::to_wstring(fps);
			std::wstring mspfStr = std::to_wstring(mspf);

			std::wstring windowText = L"Engine fps: " + fpsStr +
				L"   mspf: " + mspfStr;

			SetWindowText(static_cast<HWND>(Window.GetNativeWindow()), windowText.c_str());

			// Reset for next average.
			frameCnt = 0;
			timeElapsed += 1.0f;
		}

	}
}
