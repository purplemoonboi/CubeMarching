#include "D3D12Buffers.h"

#include "Framework/Core/Log/Log.h"
#include "Framework/Core/Time/DeltaTime.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Buffers/D3D12UploadBuffer.h"
#include "Platform/DirectX12/Materials/D3D12Material.h"


namespace Engine
{
	using namespace DirectX;

	D3D12VertexBuffer::D3D12VertexBuffer(GraphicsContext* const graphicsContext, UINT64 size, UINT vertexCount)
		:
		Layout(),
		VertexBufferByteSize(size),
		VertexCount(vertexCount)
	{
		const auto dx12GraphicsContext = dynamic_cast<D3D12Context*>(graphicsContext);

		// Reserve memory and copy the vertices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &SystemBuffer));
		CopyMemory(SystemBuffer->GetBufferPointer(), 0, size);
		
		// Create the GPU vertex buffer
		GpuBuffer = D3D12BufferUtils::CreateVertexBuffer
		(
			dx12GraphicsContext->Device.Get(),
			dx12GraphicsContext->GraphicsCmdList.Get(),
			nullptr,
			size,
			Gpu_UploadBuffer
		);
	}

	D3D12VertexBuffer::D3D12VertexBuffer(GraphicsContext* const graphicsContext, const void* vertices, UINT64 size, UINT vertexCount, bool isDynamic)
		:
		Layout(),
		VertexBufferByteSize(size),
		VertexCount(vertexCount)
	{

		const auto dx12GraphicsContext = dynamic_cast<D3D12Context*>(graphicsContext);

		// Reserve memory and copy the vertices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &SystemBuffer));
		// Copy data into buffer
		CopyMemory(SystemBuffer->GetBufferPointer(), vertices, size);

		// Create the GPU vertex buffer
		GpuBuffer = D3D12BufferUtils::CreateVertexBuffer
		(
			dx12GraphicsContext->Device.Get(),
			dx12GraphicsContext->GraphicsCmdList.Get(),
			vertices,
			size,
			Gpu_UploadBuffer
		);
	}

	//Vertex buffer methods
	void D3D12VertexBuffer::Bind() const
	{

	}

	void D3D12VertexBuffer::UnBind() const
	{

	}

	void D3D12VertexBuffer::Release()
	{

		if(SystemBuffer != nullptr)
		{
			SystemBuffer->Release();
			SystemBuffer = nullptr;
		}
		if(GpuBuffer != nullptr)
		{
			GpuBuffer->Release();
			GpuBuffer = nullptr;
		}
		if(Gpu_UploadBuffer != nullptr)
		{
			Gpu_UploadBuffer->Release();
			Gpu_UploadBuffer = nullptr;
		}
	}


	void D3D12VertexBuffer::SetData(GraphicsContext* const graphicsContext, const void* data, INT32 size)
	{
		const auto dx12GraphicsContext = dynamic_cast<D3D12Context*>(graphicsContext);

		if(GpuBuffer == nullptr)
		{
			// Reserve memory and copy the vertices into our CPU buffer
			THROW_ON_FAILURE(D3DCreateBlob(size, &SystemBuffer));
			// Copy data into buffer
			CopyMemory(SystemBuffer->GetBufferPointer(), data, size);
		}

		// Create the GPU vertex buffer
		GpuBuffer = D3D12BufferUtils::CreateVertexBuffer
		(
			dx12GraphicsContext->Device.Get(),
			dx12GraphicsContext->GraphicsCmdList.Get(),
			data,
			size,
			Gpu_UploadBuffer
		);

	}

	void D3D12VertexBuffer::SetLayout(const BufferLayout& layout)
	{
		Layout = layout;
	}

	const void* D3D12VertexBuffer::GetSystemData() const
	{
		return SystemBuffer.Get();
	}

	const void* D3D12VertexBuffer::GetResourceData() const
	{
		return GpuBuffer.Get();
	}

	D3D12_VERTEX_BUFFER_VIEW D3D12VertexBuffer::GetVertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = GpuBuffer->GetGPUVirtualAddress();
		vbv.StrideInBytes = sizeof(Vertex);
		vbv.SizeInBytes    = sizeof(Vertex) * VertexCount;

		return vbv;
	}

	D3D12IndexBuffer::D3D12IndexBuffer(GraphicsContext* const graphicsContext, UINT16* indices, UINT64 size, UINT indexCount)
		:
		Format(DXGI_FORMAT_R16_UINT),
		IndexBufferByteSize(size),
		Count(indexCount)
	{
		const auto dx12GraphicsContext = dynamic_cast<D3D12Context*>(graphicsContext);

		// Reserve memory and copy the indices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &SystemBuffer));
		CopyMemory(SystemBuffer->GetBufferPointer(), indices, size);

		// Create the GPU vertex buffer
		GpuBuffer = D3D12BufferUtils::CreateVertexBuffer
		(
			dx12GraphicsContext->Device.Get(),
			dx12GraphicsContext->GraphicsCmdList.Get(),
			indices, 
			IndexBufferByteSize, 
			Gpu_UploadBuffer
		);

	}

	//Index buffer methods
	void D3D12IndexBuffer::Bind() const
	{

	}

	void D3D12IndexBuffer::UnBind() const
	{

	}

	D3D12_INDEX_BUFFER_VIEW D3D12IndexBuffer::GetIndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation	= GpuBuffer->GetGPUVirtualAddress();
		ibv.Format			= Format;
		ibv.SizeInBytes		= Count * sizeof(UINT16);

		return ibv;
	}

	D3D12ResourceBuffer::D3D12ResourceBuffer
	(
		GraphicsContext* graphicsContext,
		const std::vector<ScopePointer<FrameResource>>& frameResources,
		UINT renderItemsCount
	)
	{
		/**	cast to appropriate api implementation */
		const auto dx12GraphicsContext = dynamic_cast<D3D12Context*>(graphicsContext);

		const auto frameResourceCount = static_cast<UINT>(frameResources.size());

		

		/**
		 * Size of the constant buffer in bytes
		 */
		const UINT64 constantBufferSizeInBytes = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));

		/**
		 * Total number of render items
		 */
		const UINT32 objectCount = renderItemsCount;

		/**
		 *	Need a CBV descriptor for each object for each frame resource.
		 */
		for (UINT32 frameIndex = 0; frameIndex < frameResourceCount; ++frameIndex)
		{
			const auto dx12FrameResource = dynamic_cast<D3D12FrameResource*>(frameResources[frameIndex].get());

			/**
			 * Get the constant buffer as a resource.
			 */
			const auto constantBuffer = dx12FrameResource->ConstantBuffer.get();

			/**
			 * Fetch the GPU address of the constant buffer
			 */
			D3D12_GPU_VIRTUAL_ADDRESS constantBufferAddress = constantBuffer->Resource()->GetGPUVirtualAddress();

			for (UINT32 i = 0; i < objectCount; ++i)
			{
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
				handle.Offset(heapIndex, dx12GraphicsContext->CbvSrvUavDescriptorSize);


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

			/**
			 * Size of the material buffer in bytes
			 */
			const UINT64 materialBufferSizeInBytes = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(MaterialConstants));
			/**
			 * Get the constant buffer as a resource.
			 */
			const auto materialBuffer = dx12FrameResource->MaterialBuffer.get();

			/**
			 * Fetch the GPU address of the constant buffer
			 */
			D3D12_GPU_VIRTUAL_ADDRESS materialBufferAddress = materialBuffer->Resource()->GetGPUVirtualAddress();

			for (UINT32 i = 0; i < objectCount; ++i)
			{
				/**
				 * Offset to the ith address in memory
				 */
				materialBufferAddress += i * materialBufferSizeInBytes;

				/**
				 * Offset to the object CBV in the heap
				 */
				const UINT32 heapIndex = frameIndex * objectCount + i;

				/**
				 * Retrieve the descriptor handle
				 */
				auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dx12GraphicsContext->CbvHeap->GetCPUDescriptorHandleForHeapStart());
				handle.Offset(heapIndex, dx12GraphicsContext->CbvSrvUavDescriptorSize);


				/**
				* Create a view for the constant buffer
				*/
				D3D12_CONSTANT_BUFFER_VIEW_DESC mbvDesc;
				mbvDesc.BufferLocation = materialBufferAddress;
				mbvDesc.SizeInBytes = materialBufferSizeInBytes;

				/**
				 * Create CBV view
				 */
				dx12GraphicsContext->Device->CreateConstantBufferView(&mbvDesc, handle);
			}


			/**
			 * After we've create a constant buffer for each render item, create the global pass buffer for
			 * the current resource.
			 */

			/**
			 * Calculate the size of the 'PassConstants' buffer in bytes
			 */
			const UINT64 passConstantBufferSizeInBytes = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(PassConstants));

			/**
			 * Create the pass buffer for this resource.
			 */
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
			handle.Offset(heapIndex, dx12GraphicsContext->CbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC passCbDesc;
			passCbDesc.BufferLocation = passConstantBufferAddress;
			passCbDesc.SizeInBytes = passConstantBufferSizeInBytes;

			dx12GraphicsContext->Device->CreateConstantBufferView(&passCbDesc, handle);

			dx12FrameResource->IsInitialised = true;
		}
	}


	void D3D12ResourceBuffer::Bind() const
	{
	}

	void D3D12ResourceBuffer::UnBind() const
	{
	}

	void D3D12ResourceBuffer::UpdatePassBuffer(const MainCamera& camera, const float deltaTime, const float elapsedTime)
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

		/**
		 *	Upload the camera data such as the position, near and far planes
		 *	and time data such as delta and elapsed time.
		 */

		MainPassConstantBuffer.EyePosW = camera.GetPosition();
		MainPassConstantBuffer.RenderTargetSize	= XMFLOAT2((float)camera.GetBufferDimensions().x, (float)camera.GetBufferDimensions().y);
		MainPassConstantBuffer.InvRenderTargetSize = XMFLOAT2(1.0f / camera.GetBufferDimensions().x, 1.0f / camera.GetBufferDimensions().y);
		MainPassConstantBuffer.NearZ = 1.0f;
		MainPassConstantBuffer.FarZ = 1000.0f;
		MainPassConstantBuffer.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
		MainPassConstantBuffer.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
		MainPassConstantBuffer.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
		MainPassConstantBuffer.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
		MainPassConstantBuffer.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
		MainPassConstantBuffer.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
		MainPassConstantBuffer.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };
		MainPassConstantBuffer.TotalTime = elapsedTime;
		MainPassConstantBuffer.DeltaTime = deltaTime;


		const auto currentPassCB = CurrentFrameResource->PassBuffer.get();
		currentPassCB->CopyData(0, MainPassConstantBuffer);
	}

	void D3D12ResourceBuffer::UpdateObjectBuffers(std::vector<RenderItem*>& renderItems)
	{

		const auto constantBuffer = CurrentFrameResource->ConstantBuffer.get();

		for(const auto& renderItem : renderItems)
		{
			
			/**
			 * Only update the constant buffer of an item if it has changed.
			 */

			if(renderItem->NumFramesDirty > 0)
			{
				const auto dx12RenderItem = dynamic_cast<D3D12RenderItem*>(renderItem);

				const XMMATRIX world = XMLoadFloat4x4(&dx12RenderItem->World);

				ObjectConstant objConst;
				XMStoreFloat4x4(&objConst.World, XMMatrixTranspose(world));

				/**
				 * Update the object's constant buffer
				 */
				constantBuffer->CopyData(dx12RenderItem->ObjectConstantBufferIndex, objConst);

				/**
				 * Decriment the flag, the current object has been updated
				 */
				dx12RenderItem->NumFramesDirty--;
				dx12RenderItem->HasBeenUpdated = true;
			}
		}
	}

	void D3D12ResourceBuffer::UpdateMaterialBuffers(std::vector<Material*>& materials)
	{
		const auto materialBuffer = CurrentFrameResource->MaterialBuffer.get();

		for (const auto& material : materials)
		{
			const auto dx12Material = dynamic_cast<D3D12Material*>(material);
			if (dx12Material->DirtyFrameCount > 0)
			{
				const XMMATRIX matTransform = XMLoadFloat4x4(&dx12Material->GetMaterialTransform());

				MaterialConstants matConstants;
				matConstants.DiffuseAlbedo = dx12Material->GetDiffuse();
				matConstants.FresnelR0 = dx12Material->GetFresnelR0();
				matConstants.Roughness = dx12Material->GetRoughness();
				XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

				materialBuffer->CopyData(dx12Material->MaterialBufferIndex, matConstants);

				// Next FrameResource need to be updated too.
				dx12Material->DirtyFrameCount--;
			}
		}
	}

	const INT32 D3D12ResourceBuffer::GetCount() const
	{

		return 0;

	}

}
