#include "cmpch.h"
#include "Application.h"

#include "Core/Log/Log.h"
#include "Core/Input/Input.h"

namespace DX12Framework
{
	Application* Application::SingletonInstance = nullptr;

	Application::Application(const std::wstring& appName)
		:
		IsRunning(true),
		IsMinimised(false)
	{
		//Check if an app instance exists
		CORE_ASSERT(!SingletonInstance, "An application instance already exists!");
		SingletonInstance = this;

		// TODO: Create a windows window

		// TODO: Initialise the renderer

	}

	void Application::Run()
	{
		
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
