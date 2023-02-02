#pragma once
#include "Framework/Core/Core.h"


namespace Engine
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

		virtual void SwapBuffers() = 0;

		static ScopePointer<GraphicsContext> Create(HWND windowHandle, INT32 swapChainBufferWidth, INT32 swapChainBufferHeight);

	};
}
