#pragma once
#include <Windows.h>

#include "FrameResource.h"
#include "Framework/Core/Core.h"
#include "Framework/Renderer/Buffers/FrameBuffer.h"
#include "Framework/Renderer/Buffers/VertexArray.h"
#include "Framework/Renderer/Engine/Mesh.h"
#include "Framework/Renderer/Engine/RendererStatus.h"
#include "Framework/Renderer/Memory/MemoryManager.h"
#include "Framework/Renderer/Pipeline/PipelineStateObject.h"

namespace Engine
{
	class Texture;

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

		virtual void Init(GraphicsContext* context, INT32 viewportWidth, INT32 viewportHeight) = 0;

		virtual void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) = 0;

		virtual void BindDepthPass() = 0;

		virtual void BindGeometryPass(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems) = 0;

		virtual void BindLightingPass() = 0;

		virtual void BindPostProcessingPass() = 0;

		virtual void ResetCommandList() = 0;

		virtual void ExecCommandList() = 0;

		virtual void UpdateFrameResource(FrameResource* frameResource) = 0;

		virtual void DrawIndexed(const RefPointer<VertexArray>& vertexArray, INT32 indexCount = 0) = 0;

		virtual void DrawIndexed(const ScopePointer<MeshGeometry>& geometry, INT32 indexCount = 0) = 0;

		virtual void PreRender() = 0;

		virtual void PostRender() = 0;

		

		virtual void Flush() = 0;

		static Api GetAPI() { return RenderingApi; }

		[[nodiscard]] virtual GraphicsContext* GetGraphicsContext() const = 0;

		[[nodiscard]] virtual FrameBuffer* GetFrameBuffer() const = 0;

		[[nodiscard]] virtual MemoryManager* GetMemoryManager() const = 0;

		[[nodiscard]] virtual FrameResource* GetCurrentFrameResource() const = 0;

	private:

		static Api RenderingApi;

	};
}


