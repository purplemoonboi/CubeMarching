#include <DX12Framework.h>
#include <Core/EntryPoint.h>

#include "DX12EditorLayer.h"


namespace DX12Framework
{
	class DX12Application : public Application
	{
	public:
		DX12Application()
			:
			Application(L"DX12 Editor")
		{
			PushLayer(new DX12EditorLayer());
		}

		~DX12Application()
		{}
	};

	Application* CreateApplication()
	{
		return new DX12Application();
	}
}
