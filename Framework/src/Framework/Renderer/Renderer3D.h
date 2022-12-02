#pragma once
#include <intsafe.h>
#include "Framework/Camera/MainCamera.h";
#include "Framework/Core/Time/AppTimeManager.h"

#include "Framework/IsoSurface/MarchingCubesSolver.h"
#include "Framework/Maths/Perlin.h"



namespace Engine
{
	class DeltaTime;

	class RendererApi;
	class GraphicsContext;

	class Renderer3D
	{
	public:

		// @brief - Resets command lists for Apis such as DirectX12 and Vulkan
		static void PreInit();

		// @brief - Initialises descriptions of vertex buffer.
		static void Init();

		// @brief - Closes command lists for Apis such as DirectX12 and Vulkan
		static void PostInit();

		// @brief - Cleans the rendering system.
		static void Shutdown();

		// @brief - Marks the start of rendering commands.
		//		
		static void BeginScene(const MainCamera& cam, const DeltaTime& appTimeManager);

		// @brief - Marks the end to capturing rendering instructions.
		//			Calls a flush() once the current block of data is
		//			calculated for rendering.
		static void EndScene();

		//TODO: THINK ABOUT MOVING THIS, DOESN'T SEEM RIGHT TO HAVE IT HERE
		// @brief - Build the frame resources for the scene
		static void BuildFrameResources(GraphicsContext* graphicsContext);

		//TODO: SIMILARLY MAYBE NOT HAVE THIS HERE
		// @brief - Build the objects which will be rendered to the buffer.
		static void BuildRenderItems(GraphicsContext* graphicsContext);


		//TODO: Temps
		static void BuildScalarField();
		static void PolygoniseScalarField();
		static void BuildMCBuffers();

		struct RenderingStats
		{
			UINT32 DrawCalls = 0;
			UINT32 TriCount = 0;
			UINT32 PolyCount = 0;

			//TODO: Implement this properly!
			UINT32 GetTotalVertexCount() { return 0; }
			//TODO: Implement this properly!
			UINT32 GetTotalIndexCount() { return 0; }
		};

		//static RenderingStats& GetRenderingStats();


	};


}
