#include "Framework/cmpch.h"
#include "Framework/Core/Log/Log.h"
#include "Win32Window.h"

#include "Framework/Core/Events/AppEvents.h"

namespace Engine
{
	Win32Window* Win32Window::WindowSPtr = nullptr;

	Win32Window* Win32Window::GetWindow()
	{
		return WindowSPtr;
	}

	Win32Window::Win32Window
	(
		HINSTANCE hInstance,
		WNDPROC wndProc,
		INT32 width, 
		INT32 height, 
		const std::wstring& windowCaption
	)
		:
		WindowHandle(nullptr),
		Width(width),
		Height(height),
		IsMinimised(false),
		IsMaximised(false),
		IsResizing(false),
		IsClosing(false)
	{
		CORE_ASSERT(!WindowSPtr, "A window already exists! You can only have '1' window registered to the system!");
		WindowSPtr = this;

		//When it's safe to do so, initialise our window parameters.
		if(!InitialiseWindow(hInstance, wndProc, windowCaption))
		{
			CORE_ERROR("A window could be initialised.... terminating application.")
		}



	}

	void Win32Window::OnUpdate()
	{
		if (Window32Data.Width > -1 && Window32Data.Height > -1)
		{
			if (Window32Data.Width != Width || Window32Data.Height != Height)
			{
				Width = Window32Data.Width;
				Height = Window32Data.Height;

				WindowResizeEvent event(Window32Data.Width, Window32Data.Height);
				Window32Data.AppEventCallBack(event);
			}
		}
	}

	bool Win32Window::InitialiseWindow(HINSTANCE hInstance, WNDPROC wndProc, const std::wstring& windowCaption)
	{
		WNDCLASS windowClass;
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = wndProc;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = hInstance;
		windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
		windowClass.hCursor = LoadCursor(0, IDC_ARROW);
		windowClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		windowClass.lpszMenuName = 0;
		windowClass.lpszClassName = L"Main Window";

		if (!RegisterClass(&windowClass))
		{
			MessageBox(0, L"Failed to register window class...", 0, 0);
			return false;
		}

		// Calculate the dimensions of the window rectangle with respect
		// to the requested width and height.
		RECT WindowRect = { 0, 0, Width, Height };
		AdjustWindowRect(&WindowRect, WS_OVERLAPPED, false);
		INT32 width = WindowRect.right - WindowRect.left;
		INT32 height = WindowRect.bottom - WindowRect.top;

		// Try and create a new window..
		WindowHandle = CreateWindow
		(
			L"Main Window",
			windowCaption.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			width,
			height,
			0,
			0,
			hInstance,
			0
		);

		if (!WindowHandle)
		{
			MessageBox(0, L"Failed to create new window...", 0, 0);
		}

		ShowWindow(WindowHandle, SW_SHOW);
		UpdateWindow(WindowHandle);

		return true;
	}

	void Win32Window::UpdateWindowData(INT32 width, INT32 height, bool isMinimised, bool isMaximised, bool isClosing, bool isResizing, bool vSync)
	{
		IsClosing = isClosing;
		IsMinimised = isMinimised;
		IsMaximised = isMaximised;
		IsResizing = isResizing;

		Window32Data.Width = width;
		Window32Data.Height = height;
	}
}
