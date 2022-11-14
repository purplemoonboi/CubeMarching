#pragma once
#include <intsafe.h>
#include "DirectX12.h"
#include "Framework/Renderer/RendererAPI.h"
#include "Platform/DirectX12/DX12GraphicsContext.h"

#include "DX12Buffer.h"
#include "DX12PipelineStateObject.h"

namespace Engine
{
	struct ObjectConstant;

	class DX12FrameBuffer;

	class DX12RenderingApi : public RendererAPI
	{
	public:
		virtual ~DX12RenderingApi();


		void Init() override;

		void InitD3D(HWND windowHandle, INT32 viewportWidth, INT32 viewportHeight) override;

		void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) override;

		void SetClearColour(const float colour[4]) override;

		void Flush() override;

		void ResetCommandList() override;

		void ExecCommandList() override;



		// @brief Adds a draw instance instruction to the command list
		// @param[in] A unique pointer to the vertex array to be submitted.
		void DrawIndexed(const RefPointer<VertexArray>& vertexArray, INT32 indexCount = 0) override{}

		// @brief Adds a draw instance instruction to the command list
		// @param[in] A unique pointer to the vertex array to be submitted.
		void DrawIndexed(const RefPointer<Geometry>& geometry, INT32 indexCount = 0, PipelineStateObject* pso = nullptr) override;

		GraphicsContext* GetGraphicsContext() const override { return GraphicsContext.get(); };

	
	private:

		// A unique pointer to the graphics context
		RefPointer<DX12GraphicsContext> GraphicsContext = nullptr;

		// A unique pointer to the framebuffer
		RefPointer<DX12FrameBuffer> FrameBuffer = nullptr;

	};

}

