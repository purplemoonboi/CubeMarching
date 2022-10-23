#pragma once
#include <intsafe.h>

#include "Framework/Core/Core.h"
#include "Framework/Camera/Camera.h";

namespace DX12Framework
{
	class DX12GraphicsContext;
	class DX12FrameBuffer;

	class Renderer3D
	{
	public:

		// @brief - Initialises descriptions of vertex buffer.
		static void Init();

		// @brief - Cleans the rendering system.
		static void Shutdown();

		// @brief - Marks the start of rendering commands.
		//			Resets the current pointer to the buffer to
		//			point at the base. 
		static void BeginScene(Camera& camera);

		// @brief - Marks the end to capturing rendering instructions.
		//			Calls a flush() once the current block of data is
		//			calculated for rendering.
		static void EndScene();

		// @brief - Creates a draw call instruction. 
		static void FlushCommandQueue();

		// @brief - For now this will render a colour to the viewport.
		static void Draw();

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

		static RenderingStats& GetRenderingStats();

	private:

	};


}
