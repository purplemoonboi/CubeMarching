#pragma once
#include <Windows.h>

#include "FrameResource.h"
#include "VertexArray.h"
#include "RenderItems.h"
#include "PipelineStateObject.h"
#include "Framework/Core/Core.h"

namespace Engine
{


	class RendererAPI
	{
	public:

		enum class Api
		{

			None = 0,
			OpenGL = 1,
			Vulkan = 2,
			DX11 = 3,
			DX12 = 4,

		};


	public:

		virtual void Init() = 0;

		virtual void InitD3D(HWND windowHandle, INT32 viewportWidth, INT32 viewportHeight) = 0;

		virtual void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) = 0;

		virtual void SetClearColour(const float colour[4], PipelineStateObject* pso) = 0;

		virtual void ResetCommandList() = 0;

		virtual void ExecCommandList() = 0;

		virtual void UpdateFrameResource(FrameResource* frameResource) = 0;

		virtual void DrawIndexed(const RefPointer<VertexArray>& vertexArray, INT32 indexCount = 0) = 0;

		virtual void DrawIndexed(const RefPointer<MeshGeometry>& geometry, INT32 indexCount = 0) = 0;

		virtual void DrawRenderItems(std::vector<RenderItem*>& renderItems, UINT currentFrameResourceIndex, UINT opaqueItemCount) = 0;

		virtual void Flush() = 0;

		static Api GetAPI() { return RenderingApi; }

		virtual GraphicsContext* GetGraphicsContext() const = 0;

	protected:

		//RefPointer<GraphicsContext> GraphicsContext;

	private:

		static Api RenderingApi;


	};
}


