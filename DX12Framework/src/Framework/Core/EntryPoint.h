#pragma once

extern DX12Framework::Application* DX12Framework::CreateApplication(HINSTANCE hInstance, const std::wstring& appName);

int WINAPI main
(
	HINSTANCE hInstance, 
	HINSTANCE prevInstance, 
	PSTR cmd, 
	INT32 showCmd
)
{

	auto app = DX12Framework::CreateApplication(hInstance, L"DX12 Engine");

	app->Run();

	delete app;
	app = nullptr;

	return 0;
}
