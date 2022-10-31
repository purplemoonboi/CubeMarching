#pragma once
#include "RendererAPI.h"
#include "Framework/Core/Core.h"
#include "Buffer.h"
#include "VertexArray.h"


namespace Engine
{

	class RenderInstruction
	{
	public:

		static void Init()
		{
			RendererApiPtr->Init();
		}

		static void InitD3D(HWND windowHandle, INT32 viewportWidth, INT32 viewportHeight)
		{
			RendererApiPtr->InitD3D(windowHandle, viewportWidth, viewportHeight);
		}

		static void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
		{
			RendererApiPtr->SetViewport(x, y, width, height);
		}

		static void SetClearColour(const float colour[4])
		{
			RendererApiPtr->SetClearColour(colour);
		}

		static void Clear()
		{
			RendererApiPtr->Clear();
		}

		static void DrawDemoScene()
		{
			RendererApiPtr->Draw();
		}

		static RendererAPI* GetApiPtr() { return RendererApiPtr; }

		static void DrawIndexed(const RefPointer<VertexArray>& vertex_array, INT32 count = 0)
		{
	
		}

	private:

		static RendererAPI* RendererApiPtr;

	};
}


