#include "DX12Buffer.h"
#include "Platform/DirectX12/DX12GraphicsContext.h"
#include "Platform/DirectX12/DX12UploadBuffer.h"

namespace Engine
{


	DX12VertexBuffer::DX12VertexBuffer(GraphicsContext* const graphicsContext, UINT size)
		:
		Layout(),
		VertexBufferByteSize(size)
	{
		const auto dx12GraphicsContext = dynamic_cast<DX12GraphicsContext*>(graphicsContext);

		// Reserve memory and copy the vertices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &VertexBufferCPU));
		// Copy data into buffer
		CopyMemory(VertexBufferCPU->GetBufferPointer(), nullptr, size);
		// Create the GPU vertex buffer
		VertexBufferGPU = DX12BufferUtils::CreateDefaultBuffer
		(
			dx12GraphicsContext->Device.Get(),
			dx12GraphicsContext->GraphicsCmdList.Get(),
			nullptr,
			size,
			VertexBufferUploader
		);
	}

	DX12VertexBuffer::DX12VertexBuffer(GraphicsContext* graphicsContext, float* vertices, UINT size)
		:
		Layout(),
		VertexBufferByteSize(size)
	{
		const auto dx12GraphicsContext = dynamic_cast<DX12GraphicsContext*>(graphicsContext);

		// Reserve memory and copy the vertices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &VertexBufferCPU));
		// Copy data into buffer
		CopyMemory(VertexBufferCPU->GetBufferPointer(), vertices, size);
		// Create the GPU vertex buffer
		VertexBufferGPU = DX12BufferUtils::CreateDefaultBuffer
		(
			dx12GraphicsContext->Device.Get(),
			dx12GraphicsContext->GraphicsCmdList.Get(),
			vertices,
			size,
			VertexBufferUploader
		);
	}

	//Vertex buffer methods
	void DX12VertexBuffer::Bind() const
	{

	}

	void DX12VertexBuffer::UnBind() const
	{

	}

	void DX12VertexBuffer::SetData(GraphicsContext* const graphicsContext, const void* data, INT32 size)
	{
		const auto dx12GraphicsContext = dynamic_cast<DX12GraphicsContext*>(graphicsContext);

		if(VertexBufferGPU == nullptr)
		{
			// Reserve memory and copy the vertices into our CPU buffer
			THROW_ON_FAILURE(D3DCreateBlob(size, &VertexBufferCPU));
			// Copy data into buffer
			CopyMemory(VertexBufferCPU->GetBufferPointer(), data, size);
		}

		// Create the GPU vertex buffer
		VertexBufferGPU = DX12BufferUtils::CreateDefaultBuffer
		(
			dx12GraphicsContext->Device.Get(),
			dx12GraphicsContext->GraphicsCmdList.Get(),
			data,
			size,
			VertexBufferUploader
		);

	}

	void DX12VertexBuffer::SetLayout(const BufferLayout& layout)
	{
		Layout = layout;
	}

	D3D12_VERTEX_BUFFER_VIEW DX12VertexBuffer::GetVertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes  = Layout.GetStride();
		vbv.SizeInBytes    = VertexBufferCPU->GetBufferSize();

		return vbv;
	}

	DX12IndexBuffer::DX12IndexBuffer(GraphicsContext* const graphicsContext, UINT16* indices, INT32 size)
		:
		Format(DXGI_FORMAT_R16_UINT),
		IndexBufferByteSize(size)
	{
		const auto dx12GraphicsContext = dynamic_cast<DX12GraphicsContext*>(graphicsContext);

		// Reserve memory and copy the indices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &IndexBufferCPU));
		CopyMemory(IndexBufferCPU->GetBufferPointer(), indices, size);

		// Create the GPU vertex buffer
		IndexBufferGPU = DX12BufferUtils::CreateDefaultBuffer
		(
			dx12GraphicsContext->Device.Get(),
			dx12GraphicsContext->GraphicsCmdList.Get(),
			indices, 
			IndexBufferByteSize, 
			IndexBufferUploader
		);

	}

	//Index buffer methods
	void DX12IndexBuffer::Bind() const
	{

	}

	void DX12IndexBuffer::UnBind() const
	{

	}

	D3D12_INDEX_BUFFER_VIEW DX12IndexBuffer::GetIndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation	= IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format			= Format;
		ibv.SizeInBytes		= IndexBufferCPU->GetBufferSize();

		return ibv;
	}

	DX12UploadBufferManager::DX12UploadBufferManager(GraphicsContext* const graphicsContext, UINT count, bool isConstant)
	{
		const auto dx12GraphicsContext = dynamic_cast<DX12GraphicsContext*>(graphicsContext);


		ConstantBuffer = CreateRef<DX12UploadBuffer<ObjectConstant>>(dx12GraphicsContext, count, isConstant);

		const UINT objectCBBytes = DX12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));

		// Get the mapped virtual address located on the GPU
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = ConstantBuffer->Resource()->GetGPUVirtualAddress();

		// Offset to the ith object in the constant buffer
		const INT32 boxCBufferIndex = 0;
		cbAddress += boxCBufferIndex * objectCBBytes;


		// Create a constant buffer view
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = DX12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));

		HRESULT h = dx12GraphicsContext->Device->GetDeviceRemovedReason();

		THROW_ON_FAILURE(h);

		dx12GraphicsContext->Device->CreateConstantBufferView
		(
			&cbvDesc,
			dx12GraphicsContext->CbvHeap->GetCPUDescriptorHandleForHeapStart()
		);


	}

	void DX12UploadBufferManager::Bind() const
	{
	}

	void DX12UploadBufferManager::UnBind() const
	{
	}

	void DX12UploadBufferManager::Update(MainCamera& camera)
	{
		//World matrix in shader
		ObjectConstant objConstants;
		XMStoreFloat4x4(&objConstants.WorldViewProj, DirectX::XMMatrixTranspose(camera.GetWorldViewProjMat()));

		ConstantBuffer->CopyData(0, objConstants);
	}

	const INT32 DX12UploadBufferManager::GetCount() const
	{

		return 0;

	}
}
