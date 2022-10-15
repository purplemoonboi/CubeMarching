#include <Framework/DX12Framework.h>
#include <Framework/Core/EntryPoint.h>

#include "DX12EditorLayer.h"


namespace DX12Framework
{
	class DX12Application : public Application
	{
	public:
		//Create a new front end application
		DX12Application(HINSTANCE hInstance, const std::wstring& appName)
			:
			Application(hInstance, appName)
		{
			PushLayer(new DX12EditorLayer());
		}

		~DX12Application()
		{}
	};

	Application* CreateApplication(HINSTANCE hInstance, const std::wstring& appName)
	{
		return new DX12Application(hInstance, appName);
	}
}
