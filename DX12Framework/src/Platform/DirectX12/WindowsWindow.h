#pragma once
#include <WindowsX.h>



namespace DX12Framework
{
	class WindowsWindow
	{
	public:

		WindowsWindow
		(
			HINSTANCE hInstance, 
			INT32 width, 
			INT32 height, 
			const std::wstring& windowCaption = L"DX12 Engine"
		);

		WindowsWindow(const WindowsWindow&) = delete;
	//	WindowsWindow& operator=(const WindowsWindow&) = delete;

		// @brief Returns the handle to the registered window.
		HWND GetWindowHandle() const { return WindowHandle; }

		// @brief Returns the maximised boolean
		bool IsWndMaximised() const { return IsMaximised; }

		// @brief Returns the minimised boolean
		bool IsWndMinimised() const { return IsMinimised; }

		// @brief Returns if the user has closed the window
		bool HasUserRequestedToCloseWnd() const { return IsClosing; }


		// @brief Event callback bound to the system, processes and handles
		//		  received events from the user.
		// @param[in] A handle to the window
		// @param[in] The event received from the system
		// @param[in] A wide parameter
		// @param[in] A long parameter
		LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	public:
		// @brief Returns the pointer to this class.
		static WindowsWindow* GetWindow();
	private:

		static WindowsWindow* WindowSPtr;

		// @brief Initialises new window parameters and registers the
		//		  window to the system.
		bool InitialiseWindow(HINSTANCE hInstance, const std::wstring& windowCaption = L"DX12 Engine");


		// @brief When the user has resized the screen, dispatch an event to the renderer.
		//		  This is so the renderer can rebuild the buffers and render targets.
		inline void OnResize();

		// Window attributes
		// Handle to the registered window
		HWND WindowHandle;

		INT32 Width;
		INT32 Height;
		bool IsMinimised;
		bool IsMaximised;
		bool IsResizing;
		bool IsClosing;
		bool WindowCreated;
	};
}


