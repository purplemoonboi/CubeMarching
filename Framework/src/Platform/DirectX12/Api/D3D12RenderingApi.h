#pragma once
#include "Framework/Renderer/Api/RendererAPI.h"
#include "D3D12Context.h"
#include "Framework/Renderer/Api/RendererAPI.h"

#include "Platform/DirectX12/Buffers/D3D12FrameBuffer.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"


namespace Engine
{
	struct ObjectConstant;

	class D3D12RenderingApi : public RendererAPI
	{
	public:


		virtual ~D3D12RenderingApi();


		void Init() override;

		void InitD3D12(HWND windowHandle, INT32 viewportWidth, INT32 viewportHeight) override;

		void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) override;

		void SetClearColour(const float colour[4], PipelineStateObject* pso) override;

		void Flush() override;

		void ResetCommandList() override;

		void ExecCommandList() override;

		void UpdateFrameResource(FrameResource* const frameResource) override;

		void DrawIndexed(const RefPointer<VertexArray>& vertexArray, INT32 indexCount = 0) override {}
		void DrawIndexed(const ScopePointer<MeshGeometry>& geometry, INT32 indexCount = 0) override {}

		void PreRender() override;

		void PostRender() override;


		// @brief 
		void DrawOpaqueItems
		(
			const std::vector<RenderItem*>& renderItems,
			UINT currentFrameResourceIndex
		) override;

		[[nodiscard]] GraphicsContext* GetGraphicsContext() const override { return D3D12Context.get(); }

		[[nodiscard]] FrameBuffer* GetFrameBuffer() const override { return D3D12FrameBuffer.get(); };


	private:
		// A unique pointer to the graphics context
		ScopePointer<D3D12Context> D3D12Context = nullptr;

		// A unique pointer to the frame buffer
		ScopePointer<D3D12FrameBuffer> D3D12FrameBuffer = nullptr;

		D3D12FrameResource* CurrentFrameResource = nullptr;

	};

}
