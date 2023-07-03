#pragma once
#include "Framework/Renderer/Buffers/Buffer.h"
#include "Framework/Primitives/GeometryBuilder.h"


#include "../Resources/D3D12RenderFrame.h"
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

		D3D12VertexBuffer(const void* vertices, UINT64 size, UINT vertexCount, bool isDynamic = false);
		DISABLE_COPY(D3D12VertexBuffer);
		D3D12VertexBuffer(D3D12VertexBuffer&& rhs) noexcept;
		auto operator=(D3D12VertexBuffer&& rhs) noexcept -> D3D12VertexBuffer&;
		~D3D12VertexBuffer() override;

		void OnDestroy() override;

		// @brief Sets the vertex data for this buffer.
		void SetData(const void* data, INT32 size, INT32 count) override;
		void SetBuffer(const void* buffer) override;

		void SetLayout(const BufferLayout& layout) override;
		[[nodiscard]] const BufferLayout& GetLayout() const override;
		[[nodiscard]] UINT32 GetCount() override;
		[[nodiscard]] const void* GetData() const override;


		[[nodiscard]] D3D12_VERTEX_BUFFER_VIEW GetView() const;

	private:
		ComPtr<ID3DBlob> Blob{ nullptr };
		ComPtr<ID3D12Resource> pVertexResource{ nullptr };
		ComPtr<ID3D12Resource> pUploadBuffers[2]{nullptr};

		BufferLayout Layout;
		BufferEventsFlags BufferState = BufferEventsFlags::Idle;

		UINT64 Size;
		UINT Count;
	};

	class D3D12IndexBuffer : public IndexBuffer
	{
	public:

		D3D12IndexBuffer(UINT16* indices, UINT64 size, UINT count);
		D3D12IndexBuffer(D3D12IndexBuffer&& rhs) noexcept;
		auto operator=(D3D12IndexBuffer&& rhs) noexcept -> D3D12IndexBuffer&;
		DISABLE_COPY(D3D12IndexBuffer);
		~D3D12IndexBuffer() override;

		void SetData(const UINT16* data, INT32 count) override;
		void SetBuffer(const void* bufferAddress) override;
		void OnDestroy() override;

		[[nodiscard]] UINT GetCount() const override;
		[[nodiscard]] UINT16* GetData() const override;
		[[nodiscard]] D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

		BufferEventsFlags BufferState = BufferEventsFlags::Idle;

	private:
		ComPtr<ID3DBlob> Blob{ nullptr };
		ComPtr<ID3D12Resource> pIndexResource{ nullptr };
		ComPtr<ID3D12Resource> pUploadResource{ nullptr };

		DXGI_FORMAT Format;

		UINT64 Size;
		UINT Count;

	};

	template<typename T>
	class D3D12StructuredBuffer : StructuredBuffer<T>
	{
	public:

		D3D12StructuredBuffer() = default;
		DISABLE_COPY(D3D12StructuredBuffer<T>);

		D3D12StructuredBuffer(T&& args) noexcept;
		auto operator=(T&& rhs) noexcept -> D3D12StructuredBuffer<T>&;

		~D3D12StructuredBuffer() override;

		void OnDestroy() override;
		void SetBuffer(const T&& other) override;

	protected:
		T Struct;

	private:

		ComPtr<ID3D12Resource> pResource{nullptr};
		ComPtr<ID3DBlob> Blob{ nullptr };

		UINT64 Size{ 0 };

	};


	
}


