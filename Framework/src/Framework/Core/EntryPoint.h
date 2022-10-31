#pragma once

extern Engine::Application* Engine::CreateApplication(HINSTANCE hInstance, const std::wstring& appName);

int WINAPI main
(
	HINSTANCE hInstance, 
	HINSTANCE prevInstance, 
	PSTR cmd, 
	INT32 showCmd
)
{

	auto app = Engine::CreateApplication(hInstance, L"DX12 Engine");

	app->Run();

	delete app;
	app = nullptr;

	return 0;
}
