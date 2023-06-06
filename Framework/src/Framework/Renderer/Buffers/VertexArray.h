#pragma once
#include "Framework/Core/Core.h"
#include "Framework/Renderer/Buffers/Buffer.h"

namespace Foundation::Graphics
{
	class VertexArray
	{
	public:
		virtual ~VertexArray() = default;

		virtual void Bind() const = 0;

		virtual void UnBind() const = 0;

		virtual void AddVertexBuffer(const RefPointer<VertexBuffer>& buffer) = 0;

		virtual void SetIndexBuffer(const RefPointer<IndexBuffer>& buffer) = 0;

		virtual const std::vector<RefPointer<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const RefPointer<IndexBuffer>& GetIndexBuffer() const = 0;

		static RefPointer<VertexArray> Create();

	};
}
