#include "Framework/cmpch.h"
#include "Application.h"

#include "Framework/Core/Log/Log.h"
#include "Framework/Core/Input/Input.h"

namespace DX12Framework
{
	Application* Application::SingletonInstance = nullptr;

	Application::Application(HINSTANCE hInstance, const std::wstring& appName)
		:
		IsRunning(true)
	{
		//Check if an app instance exists
		CORE_ASSERT(!SingletonInstance, "An application instance already exists!");
		SingletonInstance = this;

		// TODO: Create a windows window
		Window = new WindowsWindow(hInstance, 1920, 1080, L"Engine");

		// TODO: Initialise the renderer

	}

	void Application::Run()
	{
		MSG message = { 0 };

		Timer.Reset();

		while(message.message != WM_QUIT)
		{
			if(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			else
			{
				if (!Window->IsWndMinimised() && IsRunning)
				{
					Timer.Tick();

					//Update each layer
					for(Layer* layer : LayerStack)
					{
						layer->OnUpdate(Timer.GetSeconds());
					}

					//Render each layer (records cmd for GPU)
					for(Layer* layer : LayerStack)
					{
						layer->OnRender();
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





}
