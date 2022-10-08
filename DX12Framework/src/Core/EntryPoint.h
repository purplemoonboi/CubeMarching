#pragma once

extern DX12Framework::Application* DX12Framework::CreateApplication();

int main()
{

	auto app = DX12Framework::CreateApplication();

	app->Run();

	delete app;
	app = nullptr;

	return 0;
}
