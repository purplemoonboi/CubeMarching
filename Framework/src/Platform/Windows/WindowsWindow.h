#pragma once
#include <WindowsX.h>
#include "Framework/Core/Window.h"
#include "Framework/Core/Events/Event.h"

#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Foundation
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow() = default;
		explicit WindowsWindow
		(
			HINSTANCE hInstance,
			WNDPROC wndProc,
			INT32 width, 
			INT32 height, 
			const std::wstring& windowCaption = L"DX12 Foundation"
		);

		explicit WindowsWindow(const WindowProperties& winProps);

		explicit WindowsWindow(const WindowsWindow& window) noexcept;

		void OnUpdate() override;



		/*..Window Attributes..*/


		// @brief Set the applications on event process
		void SetEventCallBack(const WindowCallback& callBack) override { Window32Data.AppEventCallBack = callBack; }

	public:
		// @brief Returns the pointer to this class.
		static WindowsWindow* GetWindow();

	private:

		static WindowsWindow* WindowSPtr;

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
		bool IsWindowMinimised;
		bool IsWindowFullScreen;
		bool IsResizing;
		bool IsClosing;
		bool WindowCreated;
		bool VSync;

	public:

		void UpdateWindowData(INT32 width, INT32 height, bool isMinimised, bool isMaximised, bool isClosing, bool isResizing, bool vSync);

	};
}


