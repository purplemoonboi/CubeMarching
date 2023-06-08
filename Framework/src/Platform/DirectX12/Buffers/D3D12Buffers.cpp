#include "D3D12Buffers.h"

#include "Framework/Core/Log/Log.h"
#include "Framework/Core/Time/AppTimeManager.h"

#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"
#include "Platform/DirectX12/UploadBuffer/D3D12UploadBuffer.h"
#include "Platform/DirectX12/Materials/D3D12Material.h"
#include "Platform/DirectX12/Core/D3D12Core.h"

#include <ppl.h>


using namespace DirectX;

namespace Foundation::Graphics::D3D12
{

	D3D12VertexBuffer::D3D12VertexBuffer(UINT64 size, UINT vertexCount)
		:
		Layout(),
		VertexBufferByteSize(size),
		VertexCount(vertexCount)
	{

		// Reserve memory and copy the vertices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &Blob));
		
		// Create the GPU vertex buffer
		DefaultBuffer = D3D12BufferFactory::CreateDefaultBuffer
		(
			nullptr,
			size,
			UploadBuffer
		);
	}

	D3D12VertexBuffer::D3D12VertexBuffer(const void* vertices, UINT64 size, UINT vertexCount, bool isDynamic)
		:
		Layout(),
		VertexBufferByteSize(size),
		VertexCount(vertexCount)
	{
		// Reserve memory and copy the vertices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &Blob));

		if(vertices != nullptr)
		{
			// Copy data into buffer
			CopyMemory(Blob->GetBufferPointer(), vertices, size);
		}

		// Create the GPU vertex buffer
		DefaultBuffer = D3D12BufferFactory::CreateDefaultBuffer
		(
			vertices,
			size,
			UploadBuffer
		);
	}

	//Vertex buffer methods
	void D3D12VertexBuffer::Bind() const
	{

	}

	void D3D12VertexBuffer::UnBind() const
	{

	}

	void D3D12VertexBuffer::Destroy()
	{

		if(Blob != nullptr)
		{
			Blob.Reset();
			Blob = nullptr;
		}
		if(DefaultBuffer != nullptr)
		{
			DefaultBuffer.Reset();
			DefaultBuffer = nullptr;
		}
		if(UploadBuffer != nullptr)
		{
			UploadBuffer.Reset();
			UploadBuffer = nullptr;
		}
		
	}


	void D3D12VertexBuffer::SetData(const void* data, INT32 size, INT32 count)
	{
		if(count > VertexCount)
		{
			Blob.Reset();
			const HRESULT hr = D3DCreateBlob(size, &Blob);
			THROW_ON_FAILURE(hr);
		}

		CopyMemory(Blob->GetBufferPointer(), data, size);
		VertexBufferByteSize = size;
		VertexCount = count;
	}

	void D3D12VertexBuffer::SetBuffer(const void* buffer)
	{
		const auto rhs = (ID3D12Resource*)buffer;
		const auto desc = rhs->GetDesc();

		/*if (desc.Width > DefaultBuffer->GetDesc().Width)
		{
			DefaultBuffer.Reset();

		}
		else*/
		{
			DefaultBuffer = rhs;
		}
	}

	void D3D12VertexBuffer::SetLayout(const BufferLayout& layout)
	{
		Layout = layout;
	}

	const void* D3D12VertexBuffer::GetData() const
	{
		
		return Blob->GetBufferPointer();
	}

	const void* D3D12VertexBuffer::GetGPUResource() const
	{
		return DefaultBuffer.Get();
	}

	D3D12_VERTEX_BUFFER_VIEW D3D12VertexBuffer::GetVertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = DefaultBuffer->GetGPUVirtualAddress();
		vbv.StrideInBytes = sizeof(Vertex);
		vbv.SizeInBytes    = sizeof(Vertex) * VertexCount;

		return vbv;
	}

	D3D12IndexBuffer::D3D12IndexBuffer(UINT16* indices, UINT64 size, UINT indexCount)
		:
		Format(DXGI_FORMAT_R16_UINT),
		IndexBufferByteSize(size),
		Count(indexCount)
	{

		// Reserve memory and copy the indices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &Blob));

		if(indexCount > 0)
		{
			CopyMemory(Blob->GetBufferPointer(), indices, size);
		}

		// Create the GPU vertex buffer
		DefaultBuffer = D3D12BufferFactory::CreateDefaultBuffer
		(
			indices, 
			IndexBufferByteSize, 
			UploadBuffer
		);

	}

	//Index buffer methods
	void D3D12IndexBuffer::Bind() const
	{

	}

	void D3D12IndexBuffer::UnBind() const
	{

	}

	void D3D12IndexBuffer::SetData(const UINT16* data, INT32 count)
	{
		if (count > Count)
		{
			Blob.Reset();
			const HRESULT hr = D3DCreateBlob(count*sizeof(UINT16), &Blob);
			THROW_ON_FAILURE(hr);
		}
		if(Count != 0)
		{
			CopyMemory(Blob->GetBufferPointer(), data, count);
		}
		Count = count;
	}

	void D3D12IndexBuffer::SetBuffer(const void* bufferAddress)
	{
		const auto rhs = (ID3D12Resource*)bufferAddress;
		const auto desc = rhs->GetDesc();
		
		/*if(desc.Width > DefaultBuffer->GetDesc().Width)
		{
			DefaultBuffer.Reset();
		}
		else*/
		{
			DefaultBuffer = rhs;
		}
	}

	void D3D12IndexBuffer::Destroy()
	{
		if(DefaultBuffer != nullptr)
		{
			DefaultBuffer.Reset();
			DefaultBuffer->Release();
		}
		if(UploadBuffer != nullptr)
		{
			UploadBuffer.Reset();
			UploadBuffer->Release();
		}
	}

	UINT16* D3D12IndexBuffer::GetData() const
	{
		return reinterpret_cast<UINT16*>(Blob.Get());
	}

	D3D12_INDEX_BUFFER_VIEW D3D12IndexBuffer::GetIndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation	= DefaultBuffer->GetGPUVirtualAddress();
		ibv.Format			= Format;
		ibv.SizeInBytes		= Count * sizeof(UINT16);

		return ibv;
	}



}
