#pragma once
#include "FrameResource.h"
#include "Framework/Core/Core.h"
#include "Framework/Core/Time/AppTimeManager.h"
#include "Framework/Renderer/Buffers/FrameBuffer.h"
#include "Framework/Renderer/Buffers/VertexArray.h"
#include "Framework/Renderer/Renderer3D/Mesh.h"
#include "Framework/Renderer/Renderer3D/RendererStatus.h"
#include "Framework/Core/Memory/MemoryManager.h"
#include "Framework/Renderer/Pipeline/RenderPipeline.h"

namespace Foundation::Graphics
{
	class RenderTarget;
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

		enum class RenderLayer : UINT32
		{
			None = -0x1,
			Position = 0x0,
			Normals = 0x1,
			Albedo = 0x2,
			Specular = 0x3,
			Depth = 0x4,
			Lighting = 0x5,
			AmbientOcclusion = 0x6,
		};
		
	public:
		static Api GetAPI() { return RenderingApi; }
		
		virtual ~RendererAPI() = 0;

		virtual void PreInit() = 0;
		virtual void PostInit() = 0;
		virtual void Clean() = 0;

		virtual void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) = 0;

		virtual void OnBeginRender() = 0;
		virtual void OnEndRender() = 0;

		virtual void Flush() = 0;

		virtual void Draw(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer) = 0;
		virtual void DrawIndexed(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer) = 0;
		virtual void DrawInstanced(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer, UINT32 instanceCount) = 0;
		virtual void DrawIndexedInstanced(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, UINT32 instanceCount) = 0;


	private:
		static Api RenderingApi;

	};
}


