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

		static void BindTerrainPass(PipelineStateObject* pso, RenderItem* terrain)
		{
			RendererApiPtr->BindTerrainPass(pso, terrain);
		}

		static void BindGeometryPass(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems)
		{
			RendererApiPtr->BindStaticGeoPass(pso, renderItems);
		}

		static void ResetGraphicsCommandList()
		{
			RendererApiPtr->PreInit();
		}

		static void ExecGraphicsCommandList()
		{
			RendererApiPtr->PostInit();
		}

		static void Flush()
		{
			RendererApiPtr->Flush();
		}

		static void PreRender
		(
			const std::vector<RenderItem*>& items, const std::vector<Material*>& materials,
			RenderItem* terrain,
			const MainCamera& camera,
			const WorldSettings& settings,
			float deltaTime,
			float elapsedTime,
			bool wireframe
		)
		{
			RendererApiPtr->PreRender(items, materials, terrain, settings, camera, deltaTime, elapsedTime, wireframe);
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


