#pragma once
#include "Framework/Core/Core.h"
#include "Framework/Core/Window.h"

namespace Foundation::Graphics
{

	// @brief Base class for the graphics context class.
	//		  This is implemented per API to support unique
	//		  work flows each API offers and encourages.
	class GraphicsContext
	{
	public:
		GraphicsContext() = default;
		virtual ~GraphicsContext() = default;


		virtual void Init() = 0;
		virtual void ShutDown() = 0;
		virtual void SwapBuffers() = 0;
		virtual void OnResizeSwapChain(INT32 x, INT32 y) = 0;

		static GraphicsContext* Create(Window* window);

	};
}
