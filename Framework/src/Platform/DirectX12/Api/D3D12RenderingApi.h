#pragma once
#include "Framework/Renderer/Api/RendererAPI.h"
#include "D3D12Context.h"
#include "Platform/DirectX12/Buffers/D3D12FrameBuffer.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"


namespace Engine
{
	struct ObjectConstant;


	class D3D12RenderingApi : public RendererAPI
	{
	public:


		virtual ~D3D12RenderingApi();

		void Init(GraphicsContext* context, INT32 viewportWidth, INT32 viewportHeight) override;

		void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) override;

		void BindDepthPass() override;

		void BindGeometryPass(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems) override;

		void BindLightingPass() override;

		void BindPostProcessingPass() override;

		void Flush() override;

		void ResetCommandList() override;

		void ExecCommandList() override;

		void UpdateFrameResource(FrameResource* const frameResource) override;

		void DrawIndexed(const RefPointer<VertexArray>& vertexArray, INT32 indexCount = 0) override {}
		void DrawIndexed(const ScopePointer<MeshGeometry>& geometry, INT32 indexCount = 0) override {}

		void PreRender() override;

		void PostRender() override;


		[[nodiscard]] GraphicsContext* GetGraphicsContext() const override { return Context; }

		[[nodiscard]] FrameBuffer* GetFrameBuffer() const override { return FrameBuffer.get(); };

		[[nodiscard]] MemoryManager* GetMemoryManager() const override { return D3D12MemoryManager.get(); }

		[[nodiscard]] FrameResource* GetCurrentFrameResource() const override { return CurrentFrameResource; }
	private:
		

		// A pointer to the graphics context
		D3D12Context* Context = nullptr;

		// A unique pointer to the frame buffer
		ScopePointer<D3D12FrameBuffer> FrameBuffer = nullptr;

		// A unique pointer to the Api's memory allocator class.
		ScopePointer<D3D12MemoryManager> D3D12MemoryManager = nullptr;


		D3D12FrameResource* CurrentFrameResource = nullptr;

	};

}

