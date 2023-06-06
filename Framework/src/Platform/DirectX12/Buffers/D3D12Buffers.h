#pragma once
#include "Framework/Renderer/Buffers/Buffer.h"
#include "Framework/Primitives/GeometryBuilder.h"

#include "Platform/DirectX12/Core/D3D12Core.h"

#include "../Resources/D3D12FrameResource.h"
#include "../RenderItems/D3D12RenderItem.h"

namespace Foundation
{
	class AppTimeManager;
}

// Using namespace
using Microsoft::WRL::ComPtr;

namespace Foundation::Graphics::D3D12
{

	class D3D12Context;

	// @brief The buffer event system is exclusive for modern APIs such as Direct X12 and Vulkan.
	//	      Because it is the programmer's responsibility for ensuring safe allocation and deallocation
	//		  of GPU resources, a simple event system for scheduling commands was devised.
	enum class BufferEventsFlags : INT8
	{
		Idle = 0x0000,				// Buffer remains in this state until....
		DirtyBuffer = 0x0001,		//....a change has been requested....
		QueueDeletion = 0x0010,		//....a request for buffer deletion.
		InFlight = 0x0011			//....buffer is in use on GPU.
	};

	class D3D12VertexBuffer : public VertexBuffer
	{
	public:

		D3D12VertexBuffer(UINT64 size, UINT vertexCount);
		D3D12VertexBuffer(const void* vertices, UINT64 size, UINT vertexCount, bool isDynamic = false);
		~D3D12VertexBuffer() override = default;

		// @brief Binds this buffer for modifications.
		void Bind() const override;
		void UnBind() const override;
		void Destroy() override;

		// @brief Sets the vertex data for this buffer.
		void SetData(const void* data, INT32 size, INT32 count) override;
		void SetBuffer(const void* buffer) override;

		void SetLayout(const BufferLayout& layout) override;

		const BufferLayout& GetLayout() const override { return Layout; }

		[[nodiscard]] const void* GetData() const override;
		[[nodiscard]] const void* GetGPUResource() const override;

		[[nodiscard]] UINT32 GetCount() override { return VertexCount; }

		// @brief Returns the view into the vertex buffer;
		[[nodiscard]] D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;


		// @brief Describes how the buffer is arranged.
		BufferLayout Layout;

		// @brief A CPU copy of the buffer. 
		ComPtr<ID3DBlob> Blob = nullptr;

		// @brief Buffer to be sent to the GPU
		ComPtr<ID3D12Resource> DefaultBuffer = nullptr;

		// @brief The intermediate buffer.
		ComPtr<ID3D12Resource> UploadBuffer = nullptr;

		UINT VertexBufferByteSize;

		BufferEventsFlags BufferState = BufferEventsFlags::Idle;

	private:
		UINT VertexCount;
	};



	class D3D12IndexBuffer : public IndexBuffer
	{
	public:

		D3D12IndexBuffer(UINT16* indices, UINT64 size, UINT count);

		~D3D12IndexBuffer() override = default;

		void Bind() const override;

		void UnBind() const override;
		void SetData(const UINT16* data, INT32 count) override;
		void SetBuffer(const void* bufferAddress) override;
		void Destroy() override;


		[[nodiscard]] INT32 GetCount() const override { return Count; }
		[[nodiscard]] UINT16* GetData() const override;
		[[nodiscard]] D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

		BufferEventsFlags BufferState = BufferEventsFlags::Idle;


		INT32 Count;

		// @brief CPU copy of the index buffer
		ComPtr<ID3DBlob> Blob = nullptr;

		// @brief GPU index buffer
		ComPtr<ID3D12Resource> DefaultBuffer = nullptr;

		// @brief Intermediate index buffer
		ComPtr<ID3D12Resource> UploadBuffer = nullptr;

		// @brief Index format
		DXGI_FORMAT Format;

		UINT64 IndexBufferByteSize;


	};



	class D3D12ResourceBuffer 
	{
	public:
		~D3D12ResourceBuffer()  = default;


		//void RegisterRenderItem();

		void UpdatePassBuffer(
			D3D12FrameResource* resource,
			const MainCamera* camera,
			AppTimeManager* time,
			bool wireframe
		) ;

		void UpdateVoxelTerrain(D3D12FrameResource* resource, RenderItem* terrain);

		void UpdateObjectBuffers(D3D12FrameResource* resource, const std::vector<RenderItem*>& renderItems) ;

		void UpdateMaterialBuffers(D3D12FrameResource* resource, const std::vector<Material*>& materials) ;

		void UpdateVoxelTerrainBuffer(D3D12FrameResource* resource, RenderItem* terrain, const std::vector<Vertex>& vertices);

	private:
		PassConstants MainPassCB;

		INT32 ObjectCount = 0;

	};

	
}


