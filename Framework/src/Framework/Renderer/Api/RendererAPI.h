#pragma once
#include <Windows.h>

#include "FrameResource.h"
#include "Framework/Core/Core.h"
#include "Framework/Core/Time/AppTimeManager.h"
#include "Framework/Renderer/Buffers/FrameBuffer.h"
#include "Framework/Renderer/Buffers/VertexArray.h"
#include "Framework/Renderer/Renderer3D/Mesh.h"
#include "Framework/Renderer/Renderer3D/RendererStatus.h"
#include "Framework/Renderer/Memory/MemoryManager.h"
#include "Framework/Renderer/Pipeline/PipelineStateObject.h"

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


		virtual void Init(GraphicsContext* context, INT32 viewportWidth, INT32 viewportHeight) = 0;
		virtual void PreInit() = 0;
		virtual void PostInit() = 0;
		virtual void Clean() = 0;

		virtual void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) = 0;
		[[nodiscard]] virtual const FrameBufferSpecifications& GetViewportSpecifications() const = 0;
		
		virtual void OnUpdatePipelineResources
		(
			MainCamera* camera,
			AppTimeManager* time,
			const std::vector<RenderItem*>& items, 
			const std::vector<Material*>& materials,
			bool wireframe
		) = 0;
		virtual void OnBeginRender() = 0;
		virtual void OnEndRender() = 0;
		virtual void BindPasses() = 0;
		virtual void Flush() = 0;

		virtual void DrawSceneStaticGeometry(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems) = 0;
		virtual void DrawIndexed(const RefPointer<VertexArray>& vertexArray, INT32 indexCount = 0) = 0;
		virtual void DrawIndexed(const ScopePointer<MeshGeometry>& geometry, INT32 indexCount = 0) = 0;

	public:/*...Getters...*/
		
		[[nodiscard]] virtual GraphicsContext*		GetGraphicsContext()	const = 0;
		[[nodiscard]] virtual const FrameBuffer*	GetFrameBuffer()		const = 0;


		// @brief Returns the scene albedo texture, (unlit)
		[[nodiscard]] virtual const RenderTarget* GetSceneAlbedoTexture()				const = 0;
		// @brief Returns the scene normals in world space
		[[nodiscard]] virtual const RenderTarget* GetSceneNormalTexture()				const = 0;
		// @brief Returns the scene ambient occlusion map
		[[nodiscard]] virtual const RenderTarget* GetSceneAmbientOcclusionTexture()		const = 0;
		// @brief Returns the depth texture of the scene
		[[nodiscard]] virtual const RenderTarget* GetSceneDepthTexture()				const = 0;

	private:
		static Api RenderingApi;

	};
}


