#include "DX12Buffer.h"

#include "Framework/Core/Time/DeltaTime.h"
#include "Platform/DirectX12/DX12FrameResource.h"
#include "Platform/DirectX12/DX12GraphicsContext.h"
#include "Platform/DirectX12/DX12UploadBuffer.h"

namespace Engine
{
	using namespace DirectX;

	DX12VertexBuffer::DX12VertexBuffer(GraphicsContext* const graphicsContext, UINT64 size, UINT vertexCount)
		:
		Layout(),
		VertexBufferByteSize(size),
		VertexCount(vertexCount)
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

	DX12VertexBuffer::DX12VertexBuffer(GraphicsContext* const graphicsContext, const void* vertices, UINT64 size, UINT vertexCount)
		:
		Layout(),
		VertexBufferByteSize(size),
		VertexCount(vertexCount)
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
		vbv.StrideInBytes = sizeof(Vertex);
		vbv.SizeInBytes    = sizeof(Vertex) * VertexCount;

		return vbv;
	}

	DX12IndexBuffer::DX12IndexBuffer(GraphicsContext* const graphicsContext, UINT16* indices, UINT64 size, UINT indexCount)
		:
		Format(DXGI_FORMAT_R16_UINT),
		IndexBufferByteSize(size),
		Count(indexCount)
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
		ibv.SizeInBytes		= Count * sizeof(UINT16);

		return ibv;
	}

	DX12UploadBufferManager::DX12UploadBufferManager
	(
		GraphicsContext* const graphicsContext,
		ScopePointer<FrameResource>* const frameResources,
		UINT count, 
		bool isConstant, 
		UINT frameResourceCount, 
		UINT renderItemsCount
	)
	{
		/**	cast to appropriate api implementation */
		const auto dx12GraphicsContext = dynamic_cast<DX12GraphicsContext*>(graphicsContext);



		/**
		 * Create the descriptor heap for the constant buffer if one hasn't been created already.
		 */
		if(dx12GraphicsContext->CbvHeap == nullptr)
		{
			dx12GraphicsContext->CreateCBVAndSRVDescHeaps(renderItemsCount, frameResourceCount);
		}

		/**
		 * Size of the constant buffer in bytes
		 */
		const UINT64 constantBufferSizeInBytes = DX12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));

		/**
		 * Total number of render items
		 */
		const UINT32 objectCount = renderItemsCount;

		/**
		 *	Need a CBV descriptor for each object for each frame resource.
		 */ 
		for (UINT32 frameIndex = 0; frameIndex < frameResourceCount; ++frameIndex)
		{
			const auto dx12FrameResource = dynamic_cast<DX12FrameResource*>(frameResources[frameIndex].get());

			const auto constantBuffer = dx12FrameResource->ConstantBuffer.get();

			for (UINT32 i = 0; i < objectCount; ++i)
			{
				/**
				* Fetch the GPU address of the constant buffer
				*/
				D3D12_GPU_VIRTUAL_ADDRESS constantBufferAddress = constantBuffer->Resource()->GetGPUVirtualAddress();


				/**
				 * Offset to the ith address in memory
				 */
				constantBufferAddress += i * constantBufferSizeInBytes;

				/**
				 * Offset to the object CBV in the heap
				 */
				const UINT32 heapIndex = frameIndex * objectCount + i;

				/**
				 * Retrieve the descriptor handle 
				 */
				auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dx12GraphicsContext->CbvHeap->GetCPUDescriptorHandleForHeapStart());
				handle.Offset(heapIndex, dx12GraphicsContext->GetCbvDescSize());


				/**
				* Create a view for the constant buffer
				*/
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
				cbvDesc.BufferLocation = constantBufferAddress;
				cbvDesc.SizeInBytes = constantBufferSizeInBytes;

				/**
				 * Create CBV view
				 */
				dx12GraphicsContext->Device->CreateConstantBufferView(&cbvDesc, handle);
			}
		}

		/**
		 * Calculate the size of the 'PassConstants' buffer in bytes
		 */
		const UINT64 passConstantBufferSizeInBytes = DX12BufferUtils::CalculateConstantBufferByteSize(sizeof(PassConstants));

		/**
		 *	Last three descriptors are the pass CBVs for each frame resource.
		 */ 
		for (UINT32 frameIndex = 0; frameIndex < frameResourceCount; ++frameIndex)
		{
			const auto dx12FrameResource = dynamic_cast<DX12FrameResource*>(frameResources[frameIndex].get());

			const auto passConstantBuffer = dx12FrameResource->PassBuffer->Resource();

			/**
			* Fetch the GPU address of the constant buffer
			*/
			const D3D12_GPU_VIRTUAL_ADDRESS passConstantBufferAddress = passConstantBuffer->GetGPUVirtualAddress();

			/**
			 * Offset to the object in the heap
			 */
			const UINT32 heapIndex = dx12GraphicsContext->GetPassConstBufferViewOffset() + frameIndex;

			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dx12GraphicsContext->CbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, dx12GraphicsContext->GetCbvDescSize());

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = passConstantBufferAddress;
			cbvDesc.SizeInBytes = passConstantBufferSizeInBytes;

			dx12GraphicsContext->Device->CreateConstantBufferView(&cbvDesc, handle);
		}


		//MainPassConstantBuffer = CreateScope<PassConstants>();

	}


	void DX12UploadBufferManager::CreateMainPassConstBuffer
	(
		GraphicsContext* graphicsContext, 
		UINT32 passCount,
		UINT32 objectCount
	)
	{
		//MainPassConstantBuffer = CreateScope<PassConstants>();
	}

	void DX12UploadBufferManager::Bind() const
	{
	//	ConstantBuffer->Bind(0U, nullptr);
	}

	void DX12UploadBufferManager::UnBind() const
	{
	//	ConstantBuffer->UnBind(0U);
	}

	void DX12UploadBufferManager::Update(const MainCamera& camera, const DeltaTime& appTimeManager)
	{
		/**
		 * Get the camera's view and projection matrix
		 */
		const XMMATRIX view = XMLoadFloat4x4(&camera.GetView());
		const XMMATRIX proj = XMLoadFloat4x4(&camera.GetProjection());

		/**
		 *	Create the view matrix
		 *	Store the inverse matrices
		 *	Copy the data into the pass buffer
		 */

		const XMMATRIX viewProj	    = XMMatrixMultiply(view, proj);
		const XMMATRIX invView		= XMMatrixInverse(&XMMatrixDeterminant(view), view);
		const XMMATRIX invProj		= XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
		const XMMATRIX invViewProj  = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

		XMStoreFloat4x4(&MainPassConstantBuffer.View,			XMMatrixTranspose(view));
		XMStoreFloat4x4(&MainPassConstantBuffer.InvView,		XMMatrixTranspose(invView));
		XMStoreFloat4x4(&MainPassConstantBuffer.Proj,			XMMatrixTranspose(proj));
		XMStoreFloat4x4(&MainPassConstantBuffer.InvProj,		XMMatrixTranspose(invProj));
		XMStoreFloat4x4(&MainPassConstantBuffer.ViewProj,		XMMatrixTranspose(viewProj));
		XMStoreFloat4x4(&MainPassConstantBuffer.InvViewProj,	XMMatrixTranspose(invViewProj));

		
		MainPassConstantBuffer.EyePosW = camera.GetPosition();

		/**
		 *
		 *	Upload the camera data such as the position, near and far planes
		 *	and time data such as delta and elapsed time.
		 *
		 */

		MainPassConstantBuffer.RenderTargetSize	= XMFLOAT2((float)camera.GetBufferDimensions().x, (float)camera.GetBufferDimensions().y);
		MainPassConstantBuffer.InvRenderTargetSize = XMFLOAT2(1.0f / camera.GetBufferDimensions().x, 1.0f / camera.GetBufferDimensions().y);
		MainPassConstantBuffer.NearZ = 1.0f;
		MainPassConstantBuffer.FarZ = 1000.0f;

		//TODO: NEED TO FIX APP TIME MANAGER CLASS PREVENTING OVERRIDES FROM OVERRIDING???
		MainPassConstantBuffer.TotalTime = 0.0f;
		MainPassConstantBuffer.DeltaTime = appTimeManager;

		auto currentPassCB = CurrentFrameResource->PassBuffer.get();
		currentPassCB->CopyData(0, MainPassConstantBuffer);
	}

	void DX12UploadBufferManager::UpdateConstantBuffer(std::vector<RenderItem*> items)
	{

		const auto constantBuffer = CurrentFrameResource->ConstantBuffer.get();

		for(auto& renderItem : items)
		{

			/**
			 * Only update the constant buffer of an item if it has changed.
			 */

			if(renderItem->NumFramesDirty > 0)
			{
				const auto dx12RenderItem = dynamic_cast<DX12RenderItem*>(renderItem);

				DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&dx12RenderItem->World);

				ObjectConstant objConst;
				DirectX::XMStoreFloat4x4(&objConst.World, DirectX::XMMatrixTranspose(world));

				/**
				 * Update the object's constant buffer
				 */
				constantBuffer->CopyData(renderItem->ObjectConstantBufferIndex, objConst);

				/**
				 * Decriment the flag, the current object has been updated
				 */
				renderItem->NumFramesDirty--;
			}

		}


	}


	const INT32 DX12UploadBufferManager::GetCount() const
	{

		return 0;

	}
}
