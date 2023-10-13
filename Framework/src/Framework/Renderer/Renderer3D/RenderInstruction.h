#pragma once
#include "Framework/Renderer/Api/RendererAPI.h"
#include "Framework/Core/Time/AppTimeManager.h"

namespace Foundation::Graphics
{


	class RenderInstruction
	{
	public:

		static void Init(GraphicsContext* context)
		{
			RendererApiPtr->Init(context);
		}

		static void Clean()
		{
			RendererApiPtr->Clean();
		}

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

	public:/*...Bindings...*/
		

		static void BindGeometryPass(RenderPipeline* pso, const std::vector<RenderItem*>& renderItems)
		{
			RendererApiPtr->DrawSceneStaticGeometry(pso, renderItems);
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

		

		static void OnBeginRender()
		{
			RendererApiPtr->BeginRender();
		}

		static void DrawIndexed(const RefPointer<VertexArray>& vertexArray, INT32 count = 0)
		{
			//RendererApiPtr->DrawIndexed(vertexArray, count);
		}

		static void DrawIndexed(const ScopePointer<MeshGeometry>& geometry, INT32 count = 0)
		{
			RendererApiPtr->DrawIndexed(geometry, count);
		}

		static void OnEndRender()
		{
			RendererApiPtr->EndRender();
		}
		
	public:/*...Getters...*/
		static RendererAPI* GetApiPtr() { return RendererApiPtr; }

	private:

		static RendererAPI* RendererApiPtr;

	};
}


