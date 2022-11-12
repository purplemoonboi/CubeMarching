#pragma once
#include <intsafe.h>
#include "Framework/Camera/MainCamera.h";


namespace Engine
{

	class RendererApi;

	class Renderer3D
	{
	public:

		// @brief - Initialises descriptions of vertex buffer.
		static void Init();

		// @brief - Cleans the rendering system.
		static void Shutdown();

		// @brief - Marks the start of rendering commands.
		//		
		static void BeginScene(MainCamera& cam);

		// @brief - Marks the end to capturing rendering instructions.
		//			Calls a flush() once the current block of data is
		//			calculated for rendering.
		static void EndScene();



		//TODO: Don't use this function!
		// @brief - For now this will render a colour to the viewport.
		static void DrawDemoBox();

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
