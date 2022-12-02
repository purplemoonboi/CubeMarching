#pragma once
#include <WindowsX.h>
#include "Framework/Core/Window.h"
#include "Framework/Core/Events/Event.h"


namespace Engine
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow() = default;
		WindowsWindow
		(
			HINSTANCE hInstance,
			WNDPROC wndProc,
			INT32 width, 
			INT32 height, 
			const std::wstring& windowCaption = L"DX12 Engine"
		);

	//	WindowsWindow(const WindowsWindow&) = delete;
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



		// @brief Set the applications on event process
		void SetEventCallBack(const WindowCallback& callBack) override { WindowsData.AppEventCallBack = callBack; }

	public:
		// @brief Returns the pointer to this class.
		static WindowsWindow* GetWindow();
	private:

		static WindowsWindow* WindowSPtr;

		// @brief Initialises new window parameters and registers the
		//		  window to the system.
		bool InitialiseWindow(HINSTANCE hInstance, WNDPROC wndProc, const std::wstring& windowCaption = L"DX12 Engine");


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

	public:

		void UpdateWindowData(INT32 width, INT32 height, bool isMinimised, bool isMaximised, bool isClosing, bool isResizing, bool vSync);

	};
}


