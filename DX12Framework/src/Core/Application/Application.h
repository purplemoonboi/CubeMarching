#pragma once


namespace DX12Framework
{
	class Application
	{
	private:
		Application(const std::wstring& appName);

		virtual ~Application() = default;

	public:

		virtual Application* CreateApplication() = 0;

		void Run();
		//void OnEvent(Event& event);

		//void PushLayer(Layer* layer);
		//void PushOverlay(Layer* overlay);

		//inline Window* GetWindow() { return *Window; }

		//inline static Application* Get() { return SingletonInstance; }

		//inline ImGuiLayer() { return ImGuiLayer; }

	protected:

	private:

		static Application* SingletonInstance;

	};

	// This must be defined on client.
	Application* CreateApplication();
}

