#pragma once
#include "Framework/Renderer/Api/RendererAPI.h"

namespace Engine
{
	


	class RenderInstruction
	{
	public:

	

		static void Init(GraphicsContext* context, INT32 viewportWidth, INT32 viewportHeight)
		{
			RendererApiPtr->Init(context, viewportWidth, viewportHeight);
		}

		static void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
		{
			RendererApiPtr->SetViewport(x, y, width, height);
		}

		static void BindGeometryPass(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems)
		{
			RendererApiPtr->BindGeometryPass(pso, renderItems);
		}

		static void ResetGraphicsCommandList()
		{
			RendererApiPtr->ResetCommandList();
		}

		static void ExecGraphicsCommandList()
		{
			RendererApiPtr->ExecCommandList();
		}

		static void Flush()
		{
			RendererApiPtr->Flush();
		}

		static void UpdateFrameResource(FrameResource* frameResource)
		{
			RendererApiPtr->UpdateFrameResource(frameResource);
		}

		static void PreRender()
		{
			RendererApiPtr->PreRender();
		}

		static void PostRender()
		{
			RendererApiPtr->PostRender();
		}

		static RendererAPI* GetApiPtr() { return RendererApiPtr; }

		static void DrawIndexed(const RefPointer<VertexArray>& vertexArray, INT32 count = 0)
		{
			RendererApiPtr->DrawIndexed(vertexArray, count);
		}

		static void DrawIndexed(const ScopePointer<MeshGeometry>& geometry, INT32 count = 0)
		{
			RendererApiPtr->DrawIndexed(geometry, count);
		}



	private:

		static RendererAPI* RendererApiPtr;

	};
}


