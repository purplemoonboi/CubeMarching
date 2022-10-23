#pragma once
#include <WindowsX.h>
#include "Framework/Core/Window.h"
#include "Framework/Core/Events/Event.h"


namespace DX12Framework
{
	class WindowsWindow : public Window
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


		// @brief Returns the maximised boolean
		bool IsWndMaximised() const { return IsMaximised; }

		// @brief Returns the minimised boolean
		bool IsWndMinimised() const { return IsMinimised; }

		// @brief Returns if the user has closed the window
		bool HasUserRequestedToCloseWnd() const { return IsClosing; }


		void OnUpdate() override;

		UINT32 GetWidth() const override { return Width; }
		UINT32 GetHeight() const override { return Height; }

		void* GetNativeWindow() const override { return WindowHandle; }

		/*..Window Attributes..*/

		void SetVSync(bool enabled) override { VSync = enabled; }
		bool IsVSync() const override { return VSync; }


		// @brief Event callback bound to the system, processes and handles
		//		  received events from the user.
		// @param[in] A handle to the window
		// @param[in] The event received from the system
		// @param[in] A wide parameter
		// @param[in] A long parameter
		LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


		// @brief Set the applications on event process
		void SetEventCallBack(const WindowCallback& callBack) override { WindowsData.AppEventCallBack = callBack; }

	public:
		// @brief Returns the pointer to this class.
		static WindowsWindow* GetWindow();
	private:

		static WindowsWindow* WindowSPtr;

		// @brief Initialises new window parameters and registers the
		//		  window to the system.
		bool InitialiseWindow(HINSTANCE hInstance, const std::wstring& windowCaption = L"DX12 Engine");


		// @brief Encapsulating window data.
		struct WindowData
		{
			std::string Title;

			INT32 Width;
			INT32 Height;

			std::function<void(Event&)> AppEventCallBack;


			bool VSync;
		};

		WindowData WindowsData;

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
		bool VSync;
	};
}


