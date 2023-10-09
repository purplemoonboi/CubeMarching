#include "D3D12Buffers.h"

#include "Framework/Core/Log/Log.h"
#include "Framework/Core/Time/AppTimeManager.h"

#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Resources/D3D12RenderFrame.h"
#include "Platform/DirectX12/UploadBuffer/D3D12UploadBuffer.h"

#include <ppl.h>


using namespace DirectX;

namespace Foundation::Graphics::D3D12
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//D3D12 VERTEX BUFFER///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	D3D12VertexBuffer::D3D12VertexBuffer(const void* vertices, UINT64 size, UINT vertexCount, bool isDynamic)
		:
		Layout(),
		Size(size),
		Count(vertexCount)
	{
		// Reserve memory and copy the vertices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &Blob));

		if(vertices != nullptr)
		{
			// Copy data into buffer
			CopyMemory(Blob->GetBufferPointer(), vertices, size);
		}

		// Create the GPU vertex buffer
		pVertexResource = D3D12BufferFactory::CreateDefaultBuffer
		(
			vertices,
			size
		);
	}

	D3D12VertexBuffer::D3D12VertexBuffer(D3D12VertexBuffer&& rhs) noexcept
	{
	}

	auto D3D12VertexBuffer::operator=(D3D12VertexBuffer&& rhs) noexcept -> D3D12VertexBuffer&
	{
	}

	D3D12VertexBuffer::~D3D12VertexBuffer()
	{
	}

	const BufferLayout& D3D12VertexBuffer::GetLayout() const
	{
	}

	UINT32 D3D12VertexBuffer::GetCount()
	{
	}

	void D3D12VertexBuffer::OnDestroy()
	{

		auto* srvHeap = gD3D12Context->GetSRVHeap();
		auto* rtvHeap = gD3D12Context->GetRTVHeap();
		auto* dsvHeap = gD3D12Context->GetDSVHeap();

		gD3D12Context->DeferredRelease(pVertexResource.Get());

	}


	void D3D12VertexBuffer::SetData(const void* data, INT32 size, INT32 count)
	{
		if(count > Count)
		{
			Blob.Reset();
			const HRESULT hr = D3DCreateBlob(size, &Blob);
			THROW_ON_FAILURE(hr);
		}

		CopyMemory(Blob->GetBufferPointer(), data, size);
		Size = size;
		Count = count;
	}

	void D3D12VertexBuffer::SetBuffer(const void* buffer)
	{
		const auto rhs = static_cast<ID3D12Resource*>(const_cast<void*>(buffer));

		/*if (desc.Width > pVertexResource->GetDesc().Width)
		{
			pVertexResource.Reset();

		}
		else*/
		{
			pVertexResource = rhs;
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

	D3D12_VERTEX_BUFFER_VIEW D3D12VertexBuffer::GetView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation	= pVertexResource->GetGPUVirtualAddress();
		vbv.StrideInBytes	= sizeof(Vertex);
		vbv.SizeInBytes		= sizeof(Vertex) * Count;
		
		return vbv;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//D3D12 INDEX BUFFER////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	D3D12IndexBuffer::D3D12IndexBuffer(UINT16* indices, UINT64 size, UINT indexCount)
		:
		Format(DXGI_FORMAT_R16_UINT),
		Size(size),
		Count(indexCount)
	{

		// Reserve memory and copy the indices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &Blob));

		if(indexCount > 0)
		{
			CopyMemory(Blob->GetBufferPointer(), indices, size);
		}

		// Create the GPU vertex buffer
		pIndexResource = D3D12BufferFactory::CreateDefaultBuffer
		(
			indices, 
			Size, 
			pUploadResource
		);

	}

	D3D12IndexBuffer::D3D12IndexBuffer(D3D12IndexBuffer&& rhs) noexcept
	{



		BufferState		= rhs.BufferState;
		Size			= rhs.Size;
		Count			= rhs.Count;
		Format			= rhs.Format;
	}

	auto D3D12IndexBuffer::operator=(D3D12IndexBuffer&& rhs) noexcept -> D3D12IndexBuffer&
	{
		this->pIndexResource	= std::move(rhs.pIndexResource);
		this->pUploadResource	= std::move(rhs.pUploadResource);
		this->Blob = std::move(rhs.Blob);



		return *this;
	}

	D3D12IndexBuffer::~D3D12IndexBuffer()
	{
		gD3D12Context->DeferredRelease(pIndexResource.Get());
		gD3D12Context->DeferredRelease(pUploadResource.Get());

		Blob.Reset();
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

		pIndexResource = rhs;
		
	}

	void D3D12IndexBuffer::OnDestroy()
	{
		gD3D12Context->DeferredRelease(pIndexResource.Get());
		gD3D12Context->DeferredRelease(pUploadResource.Get());

		Blob.Reset();
	}

	UINT D3D12IndexBuffer::GetCount() const
	{
		return Count;
	}

	UINT16* D3D12IndexBuffer::GetData() const
	{
		return reinterpret_cast<UINT16*>(Blob.Get());
	}

	D3D12_INDEX_BUFFER_VIEW D3D12IndexBuffer::GetIndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation	= pIndexResource->GetGPUVirtualAddress();
		ibv.Format			= Format;
		ibv.SizeInBytes		= Count * sizeof(UINT16);
		return ibv;
	}




}
