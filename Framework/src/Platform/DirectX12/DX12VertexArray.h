#pragma once
#include "Framework/Core/Core.h"
#include "Framework/Renderer/Buffers/VertexArray.h"

namespace Engine
{
	class DX12VertexArray : public VertexArray
	{
	public:
		DX12VertexArray();

		~DX12VertexArray() override;

		void Bind() const override;

		void UnBind() const override;

		void AddVertexBuffer(const RefPointer<VertexBuffer>& vertexBuffer) override;

		void SetIndexBuffer(const RefPointer<IndexBuffer>& indexBuffer) override;

		const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return VertexBuffers; }
		const RefPointer<IndexBuffer>& GetIndexBuffer() const override { return IndexBufferPtr; }

	private:
		std::vector<RefPointer<VertexBuffer>> VertexBuffers;
		RefPointer<IndexBuffer> IndexBufferPtr;

	};
}


