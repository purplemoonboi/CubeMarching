#pragma once
#include "DirectX12.h"
#include "Framework/Core/Core.h"
#include "Framework/Renderer/Buffers/FrameBuffer.h"


namespace Engine
{
	class DX12GraphicsContext;

	class DX12FrameBuffer : public FrameBuffer
	{
	public:

		DX12FrameBuffer(const DX12FrameBuffer&) = default;
		DX12FrameBuffer(const FrameBufferSpecifications& fBufferSpecs);

		virtual ~DX12FrameBuffer() = default;


		const FrameBufferSpecifications& GetSpecifications() const override { return FrameBufferSpecs; }

		const D3D12_VIEWPORT& GetViewport() const { return ScreenViewport; }
		const D3D12_RECT& GetScissorsRect() const { return ScissorRect; }

		void Bind() override;
		void UnBind() override;
		void RebuildFrameBuffer(INT32 width, INT32 height) override;

		void ResizeFrameBuffer(RefPointer<DX12GraphicsContext> graphicsContext);

		void SetViewportDimensions(INT32 width, INT32 height) { ViewportWidth = width; ViewportHeight = height; }

	private:

		// @brief - A structure describing the buffer which we render to.
		D3D12_VIEWPORT ScreenViewport;

		// @brief - Used to specify an area of the buffer we would like to render to.
		//			Therefore culls pixels not included in the scissor rect.
		D3D12_RECT ScissorRect;

		// @brief This is out dated. Please use frame buffer class instead.   
		INT32 ViewportWidth;
		INT32 ViewportHeight;

		FrameBufferSpecifications FrameBufferSpecs;

	};

}
