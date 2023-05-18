#pragma once
#include "Framework/Core/Core.h"

#include "Framework/Renderer/Api/RendererAPI.h"


namespace Foundation
{

	class Renderer
	{
	public:
		static void Init(HWND window, INT32 bufferWidth, INT32 bufferHeight);
		static void OnWindowResize(INT32 x, INT32 y, INT32 width, INT32 height);
		static RendererAPI::Api GetAPI() { return RendererAPI::GetAPI(); }
		static RendererStatus RendererStatus();

	private:
		struct SceneData
		{
			
		};

		static SceneData* SceneData;
		static enum RendererStatus RenderStatus;


		static ScopePointer<GraphicsContext> Context;

	};
}


