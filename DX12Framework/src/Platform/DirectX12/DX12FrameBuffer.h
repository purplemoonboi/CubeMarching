#pragma once
#include "Framework/Renderer/FrameBuffer.h"


namespace DX12Framework
{
	class DX12FrameBuffer : public FrameBuffer
	{
	public:

		explicit DX12FrameBuffer(const FrameBufferSpecifications& specifications);

		~DX12FrameBuffer() override;

		// @brief Returns the data which describes the buffer we are drawing to.
		const FrameBufferSpecifications& GetSpecifications() const override { return FrameBufferSpecifications; }

		// @brief 
		void Bind() override;
		void UnBind() override;

		// @brief Rebuilds the core buffers for rendering
		void RebuildFrameBuffer(INT32 width, INT32 height) override;
		//virtual INT32 GetColourAttachmentRendererID() const override;

	private:

		FrameBufferSpecifications FrameBufferSpecifications;

	};
}


