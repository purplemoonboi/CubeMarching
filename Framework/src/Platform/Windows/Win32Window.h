#pragma once
#include <WindowsX.h>
#include "Framework/Core/Window.h"
#include "Framework/Core/Events/Event.h"

#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Foundation
{
	class Win32Window : public Window
	{
	public:
		Win32Window() = default;
		Win32Window
		(
			HINSTANCE hInstance,
			WNDPROC wndProc,
			INT32 width, 
			INT32 height, 
			const std::wstring& windowCaption = L"DX12 Foundation"
		);


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
		void SetEventCallBack(const WindowCallback& callBack) override { Window32Data.AppEventCallBack = callBack; }

	public:
		// @brief Returns the pointer to this class.
		static Win32Window* GetWindow();

	private:

		static Win32Window* WindowSPtr;

		// @brief Initialises new window parameters and registers the
		//		  window to the system.
		bool InitialiseWindow(HINSTANCE hInstance, WNDPROC wndProc, const std::wstring& windowCaption = L"Foundation <DX12>");


		// @brief Encapsulating window data.
		struct WindowData
		{
			std::string Title;

			INT32 Width;
			INT32 Height;

			std::function<void(Event&)> AppEventCallBack;


			bool VSync;
		};
		WindowData Window32Data;

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


