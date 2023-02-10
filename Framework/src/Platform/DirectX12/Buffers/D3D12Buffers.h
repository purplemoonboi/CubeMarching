#pragma once
#include "Framework/Renderer/Buffers/Buffer.h"
#include "Framework/Primitives/GeometryBuilder.h"

#include <memory>
#include "../DirectX12.h"

#include "../Resources/D3D12FrameResource.h"
#include "../RenderItems/D3D12RenderItem.h"

namespace Engine
{

	// Using namespace
	using Microsoft::WRL::ComPtr;

	class D3D12Context;

	class D3D12VertexBuffer : public VertexBuffer
	{
	public:

		D3D12VertexBuffer(GraphicsContext* const graphicsContext, UINT64 size, UINT vertexCount);
		D3D12VertexBuffer(GraphicsContext* const graphicsContext, const void* vertices, UINT64 size, UINT vertexCount, bool isDynamic = false);
		~D3D12VertexBuffer() override = default;

		// @brief Binds this buffer for modifications.
		void Bind() const override;

		// @brief Releases this buffer.
		void UnBind() const override;

		void Release() override;

		// @brief Sets the vertex data for this buffer.
		void SetData(GraphicsContext* graphicsContext, const void* data, INT32 size) override;

		void SetLayout(const BufferLayout& layout) override;

		const BufferLayout& GetLayout() const override { return Layout; }

		[[nodiscard]] const void* GetSystemData() const override;
		[[nodiscard]] const void* GetResourceData() const override;

		[[nodiscard]] UINT32 GetCount() override { return VertexCount; }

		// @brief Returns the view into the vertex buffer;
		[[nodiscard]] D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;

		// @brief Describes how the buffer is arranged.
		BufferLayout Layout;

		// @brief A CPU copy of the buffer. 
		ComPtr<ID3DBlob> SystemBuffer = nullptr;

		// @brief Buffer to be sent to the GPU
		ComPtr<ID3D12Resource> GpuBuffer = nullptr;

		// @brief The intermediate buffer.
		ComPtr<ID3D12Resource> Gpu_UploadBuffer = nullptr;

		UINT VertexBufferByteSize;

		UINT VertexCount;
	};



	class D3D12IndexBuffer : public IndexBuffer
	{
	public:

		D3D12IndexBuffer(GraphicsContext* const graphicsContext, UINT16* indices, UINT64 size, UINT count);

		~D3D12IndexBuffer() override = default;

		void Bind() const override;

		void UnBind() const override;

		[[nodiscard]] INT32 GetCount() const override { return Count; }

		[[nodiscard]] D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

	private:
		INT32 Count;

		// @brief CPU copy of the index buffer
		ComPtr<ID3DBlob> SystemBuffer = nullptr;

		// @brief GPU index buffer
		ComPtr<ID3D12Resource> GpuBuffer = nullptr;

		// @brief Intermediate index buffer
		ComPtr<ID3D12Resource> Gpu_UploadBuffer = nullptr;

		// @brief Index format
		DXGI_FORMAT Format;

		UINT64 IndexBufferByteSize;

	};


	class D3D12ResourceBuffer : public ResourceBuffer
	{
	public:

		D3D12ResourceBuffer
		(
			GraphicsContext* graphicsContext,
			const std::vector<ScopePointer<FrameResource>>& frameResources,
			UINT renderItemsCount
		);

		~D3D12ResourceBuffer() override = default;


		void Bind() const override;

		void UnBind() const override;

		void UpdatePassBuffer(const MainCamera& camera, const float deltaTime, const float elapsedTime) override;

		void UpdateObjectBuffers(std::vector<RenderItem*>& renderItems) override;

		void UpdateMaterialBuffers(std::vector<Material*>& materials) override;

		const INT32 GetCount() const override;


		// @brief - Updates the current frame resource.
		void UpdateCurrentFrameResource(FrameResource* frameResource) override { CurrentFrameResource = dynamic_cast<D3D12FrameResource*>(frameResource); }

		// @brief - Returns a raw pointer to the frame resource.
		FrameResource* GetCurrentFrameResource() const override { return CurrentFrameResource; }

	private:

		// @brief - Keeps track of the current frame resource being updated.
		D3D12FrameResource* CurrentFrameResource = nullptr;


		// @brief - Main pass buffer for data such as camera data, time and additional matrix data.
		PassConstants MainPassConstantBuffer;


	};

	
}


