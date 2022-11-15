#pragma once
#include "RendererAPI.h"

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

		static void SetClearColour(const float colour[4], PipelineStateObject* pso)
		{
			RendererApiPtr->SetClearColour(colour, pso);
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

		static void DrawIndexed(const RefPointer<MeshGeometry>& geometry, INT32 count = 0)
		{
			RendererApiPtr->DrawIndexed(geometry, count);
		}

		static void DrawRenderItems(FrameResource* const frameResource, std::vector<RefPointer<RenderItem>>& renderItems, UINT currentFrameResourceIndex = 0, UINT opaqueItemCount = 0)
		{
			RendererApiPtr->DrawRenderItems(frameResource, renderItems, currentFrameResourceIndex, opaqueItemCount);
		}

	private:

		static RendererAPI* RendererApiPtr;

	};
}


