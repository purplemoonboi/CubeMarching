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

		static void Flush()
		{
			RendererApiPtr->Flush();
		}

		static void ResetGraphicsCommandList()
		{
			RendererApiPtr->ResetCommandList();
		}

		static void ExecGraphicsCommandList()
		{
			RendererApiPtr->ExecCommandList();
		}

		static void DrawDemoScene()
		{
		//	RendererApiPtr->Draw();
		}

		static RendererAPI* GetApiPtr() { return RendererApiPtr; }

		static void DrawIndexed(const RefPointer<VertexArray>& vertexArray, INT32 count = 0)
		{
			RendererApiPtr->DrawIndexed(vertexArray, count);
		}

		static void DrawIndexed(const RefPointer<Geometry>& geometry, INT32 count = 0, PipelineStateObject* pso = nullptr)
		{
			RendererApiPtr->DrawIndexed(geometry, count, pso);
		}

	private:

		static RendererAPI* RendererApiPtr;

	};
}


