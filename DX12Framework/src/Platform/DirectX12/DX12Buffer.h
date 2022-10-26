#pragma once
#include "Framework/Renderer/Buffer.h"


namespace DX12Framework
{
	class DX12VertexBuffer : public VertexBuffer
	{
		DX12VertexBuffer(INT32 size);
		DX12VertexBuffer(float* vertices, INT32 size);
		
	public:
		virtual ~DX12VertexBuffer() = default;


		void Bind() const override;
		void UnBind() const override;
		void SetData(const void* data, INT32 size) override;
		inline void SetLayout(const BufferLayout& layout) override;
		const BufferLayout& GetLayout() const override {};

	private:


	};

	class DX12IndexBuffer : public IndexBuffer
	{
		DX12IndexBuffer(INT32* indices, INT32 size);

	public:
		virtual ~DX12IndexBuffer() = default;

		void Bind() const override;

		void UnBind() const override;

		const INT32 GetCount() override { return Count; }

	private:
		INT32 Count;

	};

}


