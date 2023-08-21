#include "Window.h"

#include "Platform/Windows/WindowsWindow.h"

namespace Foundation
{
	Window* Window::Create(const WindowProperties& props)
	{
		//TODO: Implement a switch to create platform specific windows.
		return new WindowsWindow(props);
	}

	Window& Window::Create(const WindowProperties& props, bool isVsync)
	{
		//TODO: Implement a switch to create platform specific windows.
		return WindowsWindow(props);
	}

}