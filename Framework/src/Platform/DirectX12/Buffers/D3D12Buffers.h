#pragma once
#include "Framework/Renderer/Buffers/Buffer.h"
#include "Framework/Primitives/GeometryBuilder.h"

#include <memory>
#include "../DirectX12.h"

#include "../Resources/D3D12FrameResource.h"
#include "../RenderItems/D3D12RenderItem.h"

namespace Engine
{
	struct WorldSettings;

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

		void Destroy() override;

		// @brief Sets the vertex data for this buffer.
		void SetData(const void* data, INT32 size, INT32 count) override;

		void SetLayout(const BufferLayout& layout) override;

		const BufferLayout& GetLayout() const override { return Layout; }

		[[nodiscard]] const void* GetData() const override;
		[[nodiscard]] const void* GetGPUResource() const override;

		[[nodiscard]] UINT32 GetCount() override { return VertexCount; }

		// @brief Returns the view into the vertex buffer;
		[[nodiscard]] D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;

		bool Regenerate();

		// @brief Describes how the buffer is arranged.
		BufferLayout Layout;

		// @brief A CPU copy of the buffer. 
		ComPtr<ID3DBlob> CpuLocalCopy = nullptr;

		// @brief Buffer to be sent to the GPU
		ComPtr<ID3D12Resource> GpuBuffer = nullptr;

		// @brief The intermediate buffer.
		ComPtr<ID3D12Resource> Gpu_UploadBuffer = nullptr;

		UINT VertexBufferByteSize;
		INT8 Flag = 0;

	private:

		UINT VertexCount;
		std::vector<Vertex> Data;
	};



	class D3D12IndexBuffer : public IndexBuffer
	{
	public:

		D3D12IndexBuffer(UINT16* indices, UINT64 size, UINT count);

		~D3D12IndexBuffer() override = default;

		void Bind() const override;

		void UnBind() const override;
		void SetData(const UINT16* data, UINT count) override;
		void Destroy() override;

		bool Regenerate();

		[[nodiscard]] INT32 GetCount() const override { return Count; }
		[[nodiscard]] UINT16* GetData() const override;
		[[nodiscard]] D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

	private:
		INT32 Count;

		// @brief CPU copy of the index buffer
		ComPtr<ID3DBlob> CpuData = nullptr;

		// @brief GPU index buffer
		ComPtr<ID3D12Resource> DefaultBuffer = nullptr;

		// @brief Intermediate index buffer
		ComPtr<ID3D12Resource> UploadBuffer = nullptr;

		// @brief Index format
		DXGI_FORMAT Format;

		UINT64 IndexBufferByteSize;

		std::vector<UINT16> Data;

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

		void UpdatePassBuffer(
			D3D12FrameResource* resource,
			const WorldSettings& settings,
			const MainCamera& camera, 
			const float deltaTime, 
			const float elapsedTime, 
			bool wireframe
		) ;

		void UpdateObjectBuffers(D3D12FrameResource* resource, const std::vector<RenderItem*>& renderItems) ;

		void UpdateMaterialBuffers(D3D12FrameResource* resource, const std::vector<Material*>& materials) ;


		const INT32 GetCount() const ;


	private:


		// @brief - Main pass buffer for data such as camera data, time and additional matrix data.
		PassConstants MainPassConstantBuffer;

		INT32 ObjectCount = 0;


	};

	
}


