#pragma once
#include "Framework/Renderer/Api/RendererAPI.h"

namespace Engine
{
	class RenderPipeline;
	class VertexBuffer;
	class IndexBuffer;

	class RenderInstruction
	{
	public:

		static void PreInitRenderer()
		{
			RendererApiPtr->PreInit();
		}

		static void PostInitRenderer()
		{
			RendererApiPtr->PostInit();
		}

		static void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
		{
			RendererApiPtr->SetViewport(x, y, width, height);
		}

		static void OnBeginResourceCreation()
		{
			RendererApiPtr->PreInit();
		}

		static void OnEndResourceCreation()
		{
			RendererApiPtr->PostInit();
		}

		static void Flush()
		{
			RendererApiPtr->Flush();
		}

		static void Draw(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer)
		{
			RendererApiPtr->Draw(renderPipeline, vertexBuffer);
		}

		static void DrawIndexed(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer)
		{
			RendererApiPtr->DrawIndexed(renderPipeline, vertexBuffer, indexBuffer);
		}

		static void DrawInstanced(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer, UINT32 instanceCount)
		{
			RendererApiPtr->DrawInstanced(renderPipeline, vertexBuffer, instanceCount);
		}

		static void DrawIndexedInstanced(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, UINT32 instanceCount)
		{
			RendererApiPtr->DrawIndexedInstanced(renderPipeline, vertexBuffer, indexBuffer);
		}

		static ScopePointer<RendererAPI> GetApiPtr() { return RendererApiPtr; }

	private:

		static ScopePointer<RendererAPI> RendererApiPtr;

	};
}


