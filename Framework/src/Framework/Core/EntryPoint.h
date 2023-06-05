#pragma once

extern Foundation::Application* Foundation::CreateApplication(HINSTANCE hInstance, const std::wstring& appName);

int WINAPI main
(
	HINSTANCE hInstance, 
	HINSTANCE prevInstance, 
	PSTR cmd, 
	INT32 showCmd
)
{

	auto app = Foundation::CreateApplication(hInstance, L"Foundation Engine <DX12>");
	app->Run();

	delete app;
	app = nullptr;

	return 0;
}
