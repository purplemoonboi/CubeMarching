#include "Framework/cmpch.h"
#include "Framework/Core/Log/Log.h"
#include "WindowsWindow.h"

#include "Framework/Core/Events/AppEvents.h"

namespace Engine
{
	WindowsWindow* WindowsWindow::WindowSPtr = nullptr;

	WindowsWindow* WindowsWindow::GetWindow()
	{
		return WindowSPtr;
	}

	LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return WindowsWindow::GetWindow()->MsgProc(hwnd, msg, wParam, lParam);
	}

	WindowsWindow::WindowsWindow
	(
		HINSTANCE hInstance, 
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
		if(!InitialiseWindow(hInstance, windowCaption))
		{
			CORE_ERROR("A window could be initialised.... terminating application.")
		}

	}

	void WindowsWindow::OnUpdate()
	{
		if(WindowsData.Width != Width || WindowsData.Height != Height)
		{
			Width = WindowsData.Width;
			Height = WindowsData.Height;

			WindowResizeEvent event(WindowsData.Width, WindowsData.Height);
			WindowsData.AppEventCallBack(event);

		}
	}

	bool WindowsWindow::InitialiseWindow(HINSTANCE hInstance, const std::wstring& windowCaption)
	{
		WNDCLASS windowClass;
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = MainWndProc;
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

	LRESULT WindowsWindow::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{


		switch(msg)
		{
		case WM_QUIT:
			IsClosing = true;
			return 0;
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
			{
			//	Timer.Stop();
			}
			else
			{
			//	mTimer.Start();
			}
			return 0;
		case WM_SIZE:
			// Save the new client area dimensions.
			WindowsData.Width  = LOWORD(lParam);
			WindowsData.Height = HIWORD(lParam);


			if (wParam == SIZE_MINIMIZED)
			{
				IsMinimised = true;
				IsMaximised = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				IsMinimised = false;
				IsMaximised = true;

			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (IsMinimised)
				{
					IsMinimised = false;

				}

				// Restoring from maximized state?
				else if (IsMaximised)
				{
					IsMaximised = false;
				}
				else if (IsResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // Api call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{

				}

				
			}
			return 0;
			// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
		case WM_ENTERSIZEMOVE:
			IsResizing = true;
			//mTimer.Stop();
			return 0;

			// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
			// Here we reset everything based on the new window dimensions.
		case WM_EXITSIZEMOVE:
			IsResizing = false;
			//mTimer.Start();
			
			return 0;

			// WM_DESTROY is sent when the window is being destroyed.
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

			// The WM_MENUCHAR message is sent when a menu is active and the user presses 
			// a key that does not correspond to any mnemonic or accelerator key. 
		case WM_MENUCHAR:
			// Don't beep when we alt-enter.
			return MAKELRESULT(0, MNC_CLOSE);

			// Catch this message so to prevent the window from becoming too small.
		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			return 0;

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			//OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			CORE_TRACE("Mouse button down")
			return 0;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			//OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			CORE_TRACE("Mouse button up")

			return 0;
		case WM_MOUSEMOVE:
			//OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			CORE_TRACE("Mouse moved")

			return 0;
		case WM_KEYUP:
			if (wParam == VK_ESCAPE)
			{
				PostQuitMessage(0);
			}
			else if ((int)wParam == VK_F2)
				//Set4xMsaaState(!m4xMsaaState);

			return 0;
		}



		return DefWindowProc(hwnd, msg, wParam, lParam);
	}



}
