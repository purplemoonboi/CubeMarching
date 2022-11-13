#pragma once
#include "Framework/Renderer/Buffer.h"
#include "Framework/Primitives/GeometryBase.h"
#include "DX12UploadBuffer.h"
#include "DirectX12.h"

#include <memory>

#include "DX12FrameResource.h"
#include "DX12Mesh.h"

namespace Engine
{

	// Using namespace
	using Microsoft::WRL::ComPtr;

	class DX12GraphicsContext;

	class DX12VertexBuffer : public VertexBuffer
	{
	public:

		DX12VertexBuffer(GraphicsContext* const graphicsContext, UINT size);
		DX12VertexBuffer(GraphicsContext* const graphicsContext, const void* vertices, UINT size);
		~DX12VertexBuffer() override = default;

		// @brief Binds this buffer for modifications.
		void Bind() const override;

		// @brief Releases this buffer.
		void UnBind() const override;

		// @brief Sets the vertex data for this buffer.
		void SetData(GraphicsContext* graphicsContext, const void* data, INT32 size) override;

		inline void SetLayout(const BufferLayout& layout) override;

		const BufferLayout& GetLayout() const override { return Layout; }


		// @brief Returns the view into the vertex buffer;
		D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;

		// @brief Describes how the buffer is arranged.
		BufferLayout Layout;

		// @brief A CPU copy of the buffer. 
		ComPtr<ID3DBlob> VertexBufferCPU = nullptr;

		// @brief Buffer to be sent to the GPU
		ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;

		// @brief The intermediate buffer.
		ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;

		UINT VertexBufferByteSize;
	};



	class DX12IndexBuffer : public IndexBuffer
	{
	public:

		DX12IndexBuffer(GraphicsContext* const graphicsContext, UINT16* indices, UINT size, UINT count);

		~DX12IndexBuffer() override = default;

		void Bind() const override;

		void UnBind() const override;

		const INT32 GetCount() override { return Count; }

		D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

	private:
		UINT Count;

		// @brief CPU copy of the index buffer
		ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

		// @brief GPU index buffer
		ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

		// @brief Intermediate index buffer
		ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

		// @brief Index format
		DXGI_FORMAT Format;

		UINT IndexBufferByteSize;

	};



	class DX12UploadBufferManager : public UploadBuffer
	{
	public:

		DX12UploadBufferManager(GraphicsContext* const graphicsContext, UINT count, bool isConstant);

		~DX12UploadBufferManager() override = default;

		void Bind() const override;

		void UnBind() const override;

		void Update(MainCamera& camera) override;

		const INT32 GetCount() const override;

		
	private:
		RefPointer<DX12UploadBuffer<ObjectConstant>> ConstantBuffer;
	};

}


