#pragma once
#include "Framework/Core/Core.h"

#include "Framework/Renderer/Api/RendererAPI.h"


namespace Foundation
{
	class Window;
}

namespace Foundation::Graphics
{

	class Renderer
	{
	public:
		static void Init(Window* window);
		static void OnWindowResize(INT32 x, INT32 y, INT32 width, INT32 height);
		static RendererAPI::Api GetAPI() { return RendererAPI::GetAPI(); }
		static RendererStatus RendererStatus();
		static void Clean();
	private:
		
		static enum RendererStatus RenderStatus;
	};
}


