#pragma once
#include "Framework/cmpch.h"
#include "Events/Event.h"

namespace Foundation
{
	struct WindowProperties
	{
		std::wstring Title;
		UINT32 Width;
		UINT32 Height;


		WindowProperties
		(
			const std::wstring& windowTitle = L"DX12 Foundation",
			UINT32 windowWidth = 1920,
			UINT32 windowHeight = 1080
		)
			: Title(windowTitle), Width(windowWidth), Height(windowHeight)
		{}
	};


	class  Window
	{
	public:
		using WindowCallback = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		[[nodiscard]] virtual INT32 GetWidth() const = 0;
		[[nodiscard]] virtual INT32 GetHeight() const = 0;
		[[nodiscard]] virtual bool IsVSync() const = 0;
		[[nodiscard]] virtual void* GetNativeWindow() const = 0;

		[[nodiscard]] virtual bool IsMinimised() const = 0;
		[[nodiscard]] virtual bool IsFullScreen() const = 0;

		/*..Window Attributes..*/
		
		virtual void SetEventCallBack(const WindowCallback& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;

		static Window* Create(const WindowProperties& props = WindowProperties());

		
	};

}