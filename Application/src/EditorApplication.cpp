#include <Framework/Engine.h>
#include <Framework/Core/EntryPoint.h>
#include "EditorLayer.h"


namespace Engine
{
	class EditorApplication : public Application
	{
	public:
		EditorApplication(const EditorApplication&) = delete;
		EditorApplication& operator=(const EditorApplication&) = delete;

		//Create a new front end application
		EditorApplication(HINSTANCE hInstance, const std::wstring& appName)
			:
			Application(hInstance, appName)
		{
			PushLayer(new EditorLayer());
		}

		~EditorApplication()
		{

		}
	};

	Application* CreateApplication(HINSTANCE hInstance, const std::wstring& appName)
	{
		return new EditorApplication(hInstance, appName);
	}
}
