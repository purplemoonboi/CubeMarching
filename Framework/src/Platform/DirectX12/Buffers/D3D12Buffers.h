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
	class D3D12MemoryManager;

	class D3D12VertexBuffer : public VertexBuffer
	{
	public:

		D3D12VertexBuffer(UINT64 size, UINT vertexCount);
		D3D12VertexBuffer(const void* vertices, UINT64 size, UINT vertexCount, bool isDynamic = false);
		~D3D12VertexBuffer() override = default;

		// @brief Binds this buffer for modifications.
		void Bind() const override;

		// @brief Releases this buffer.
		void UnBind() const override;

		void Release() override;

		// @brief Sets the vertex data for this buffer.
		void SetData(const void* data, INT32 size) override;

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
		ComPtr<ID3DBlob> CpuLocalCopy = nullptr;

		// @brief Buffer to be sent to the GPU
		ComPtr<ID3D12Resource> GpuBuffer = nullptr;

		// @brief The intermediate buffer.
		ComPtr<ID3D12Resource> Gpu_UploadBuffer = nullptr;

		UINT VertexBufferByteSize;

		UINT VertexCount;

		INT8 Flag = 0;
	};



	class D3D12IndexBuffer : public IndexBuffer
	{
	public:

		D3D12IndexBuffer(UINT16* indices, UINT64 size, UINT count);

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


	class D3D12ResourceBuffer //: public ResourceBuffer
	{
	public:

		D3D12ResourceBuffer
		(
			ID3D12Device* device,
			D3D12MemoryManager* memoryManager,
			const std::vector<ScopePointer<D3D12FrameResource>>& frameResources,
			UINT renderItemsCount
		);

		~D3D12ResourceBuffer()  = default;


		//void RegisterRenderItem();

		void UpdatePassBuffer(D3D12FrameResource* resource, const MainCamera& camera, const float deltaTime, const float elapsedTime, bool wireframe) ;

		void UpdateObjectBuffers(D3D12FrameResource* resource, const std::vector<RenderItem*>& renderItems) ;

		void UpdateMaterialBuffers(D3D12FrameResource* resource, const std::vector<Material*>& materials) ;


		const INT32 GetCount() const ;


	private:


		// @brief - Main pass buffer for data such as camera data, time and additional matrix data.
		PassConstants MainPassConstantBuffer;

		INT32 ObjectCount = 0;


	};

	
}


