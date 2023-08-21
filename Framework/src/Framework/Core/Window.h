#pragma once
#include "Framework/cmpch.h"
#include "Events/Event.h"

namespace Foundation
{
	struct WindowProperties
	{
		std::wstring Title;
		INT32 Width;
		INT32 Height;
		HWND WinHandle;
		HINSTANCE WinInstance;
		WNDPROC WinProc;
		bool IsVsync;

		WindowProperties
		(         
			const std::wstring& windowTitle = L"Foundation Engine <DX12>",
			INT32 windowWidth = 1920,
			INT32 windowHeight = 1080,
			HWND hwnd = 0,
			HINSTANCE hInstance = nullptr,
			WNDPROC wndProc = nullptr
		)
			:	  Title(windowTitle)
				, Width(windowWidth)
				, Height(windowHeight)
				, WinHandle(hwnd)
				, WinInstance(hInstance)
				, WinProc(wndProc)
				, IsVsync(false)
		{}
	};


	class  Window
	{
	public:
		using WindowCallback = std::function<void(Event&)>;

		static Window* Create(const WindowProperties& props = WindowProperties());
		static Window& Create(const WindowProperties& props, bool isVsync = false);

		virtual ~Window() {}


		[[nodiscard]] INT32 GetWidth()				const { return	m_Width;		}
		[[nodiscard]] INT32 GetHeight()				const { return	m_Height;		}
		[[nodiscard]] bool	IsVSync()				const { return	m_IsVsync;		}
		[[nodiscard]] void* GetNativeWindow()		const { return	m_NativeWindow; }

		[[nodiscard]] virtual bool IsMinimised()	const {return m_IsMinimised;}
		[[nodiscard]] virtual bool IsFullScreen()	const {return m_IsMaximised;}

		/*..Window Attributes..*/
		
		virtual void SetVSync(bool enabled) { m_IsVsync = enabled; }

		virtual void OnUpdate() = 0;
		virtual void SetEventCallBack(const WindowCallback& callback) = 0;

	protected:
		INT32	m_Width{ -1 }, m_Height{ -1 };
		bool	m_IsVsync{false};
		void*	m_NativeWindow{nullptr};
		bool	m_IsMinimised{false};
		bool	m_IsMaximised{false};
	};

}