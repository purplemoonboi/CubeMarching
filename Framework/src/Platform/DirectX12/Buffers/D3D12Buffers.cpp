#include "D3D12Buffers.h"

#include "Framework/Core/Log/Log.h"
#include "Framework/Scene/WorldSettings.h"

#include "Framework/Core/Time/DeltaTime.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include "Platform/DirectX12/Buffers/D3D12UploadBuffer.h"
#include "Platform/DirectX12/Materials/D3D12Material.h"


namespace Engine
{
	using namespace DirectX;

	D3D12VertexBuffer::D3D12VertexBuffer(UINT64 size, UINT vertexCount)
		:
		Layout(),
		VertexBufferByteSize(size),
		VertexCount(vertexCount)
	{

		// Reserve memory and copy the vertices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &CpuLocalCopy));
		CopyMemory(CpuLocalCopy->GetBufferPointer(), 0, size);
		
		// Create the GPU vertex buffer
		GpuBuffer = D3D12BufferUtils::CreateDefaultBuffer
		(
			nullptr,
			size,
			Gpu_UploadBuffer
		);
	}

	D3D12VertexBuffer::D3D12VertexBuffer(const void* vertices, UINT64 size, UINT vertexCount, bool isDynamic)
		:
		Layout(),
		VertexBufferByteSize(size),
		VertexCount(vertexCount)
	{


		// Reserve memory and copy the vertices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &CpuLocalCopy));
		// Copy data into buffer
		CopyMemory(CpuLocalCopy->GetBufferPointer(), vertices, size);

		// Create the GPU vertex buffer
		GpuBuffer = D3D12BufferUtils::CreateDefaultBuffer
		(
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

	void D3D12VertexBuffer::Destroy()
	{

		if(CpuLocalCopy != nullptr)
		{
			CpuLocalCopy.Reset();
			CpuLocalCopy = nullptr;
		}
		if(GpuBuffer != nullptr)
		{
			GpuBuffer.Reset();
			GpuBuffer = nullptr;
		}
		if(Gpu_UploadBuffer != nullptr)
		{
			Gpu_UploadBuffer.Reset();
			Gpu_UploadBuffer = nullptr;
		}
		
	}


	void D3D12VertexBuffer::SetData(const void* data, INT32 size, INT32 count)
	{
		Data.clear();

		VertexBufferByteSize = size;
		VertexCount = count;

		Data.reserve(count);

		const auto* vertices = static_cast<Vertex*>(const_cast<void*>(data));
		for (INT32 i = 0; i < count; ++i)
			Data.push_back(vertices[i]);

	}

	void D3D12VertexBuffer::SetLayout(const BufferLayout& layout)
	{
		Layout = layout;
	}

	const void* D3D12VertexBuffer::GetData() const
	{
		return CpuLocalCopy.Get();
	}

	const void* D3D12VertexBuffer::GetGPUResource() const
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

	bool D3D12VertexBuffer::Regenerate()
	{
		// Create the GPU vertex buffer
		if(GpuBuffer != nullptr)
		{
			GpuBuffer.Reset();
			Gpu_UploadBuffer.Reset();
		}

		GpuBuffer = D3D12BufferUtils::CreateDefaultBuffer
		(
			Data.data(),
			VertexBufferByteSize,
			Gpu_UploadBuffer
		);

		if (GpuBuffer == nullptr)
			return false;

		return true;
	}

	D3D12IndexBuffer::D3D12IndexBuffer(UINT16* indices, UINT64 size, UINT indexCount)
		:
		Format(DXGI_FORMAT_R16_UINT),
		IndexBufferByteSize(size),
		Count(indexCount)
	{

		// Reserve memory and copy the indices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(size, &CpuData));
		CopyMemory(CpuData->GetBufferPointer(), indices, size);

		// Create the GPU vertex buffer
		DefaultBuffer = D3D12BufferUtils::CreateDefaultBuffer
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

	void D3D12IndexBuffer::SetData(const UINT16* data, UINT count)
	{
		Data.clear();

		Count = count;
		IndexBufferByteSize = sizeof(UINT16) * count;

		Data.reserve(count);
		for (INT32 i = 0; i < count; ++i)
			Data.push_back(data[i]);
	}

	void D3D12IndexBuffer::Destroy()
	{
		if(DefaultBuffer != nullptr)
		{
			DefaultBuffer->Release();
			DefaultBuffer = nullptr;
		}
		if(UploadBuffer != nullptr)
		{
			UploadBuffer->Release();
			UploadBuffer = nullptr;
		}
	}

	bool D3D12IndexBuffer::Regenerate()
	{
		// Reserve memory and copy the indices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(IndexBufferByteSize, &CpuData));
		CopyMemory(CpuData->GetBufferPointer(), Data.data(), IndexBufferByteSize);

		// Create the GPU vertex buffer
		DefaultBuffer = D3D12BufferUtils::CreateDefaultBuffer
		(
			Data.data(),
			IndexBufferByteSize,
			UploadBuffer
		);

		if (DefaultBuffer == nullptr)
			return false;

		return true;
	}

	UINT16* D3D12IndexBuffer::GetData() const
	{
		return reinterpret_cast<UINT16*>(CpuData.Get());
	}

	D3D12_INDEX_BUFFER_VIEW D3D12IndexBuffer::GetIndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation	= DefaultBuffer->GetGPUVirtualAddress();
		ibv.Format			= Format;
		ibv.SizeInBytes		= Count * sizeof(UINT16);

		return ibv;
	}

	D3D12ResourceBuffer::D3D12ResourceBuffer
	(
		ID3D12Device* device,
		D3D12MemoryManager* memoryManager,
		const std::vector<ScopePointer<D3D12FrameResource>>& frameResources,
		UINT renderItemsCount
	)
	{
		/**	cast to appropriate api implementation */

		const auto frameResourceCount = static_cast<UINT>(frameResources.size());

		const UINT64 constantBufferSizeInBytes = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));

		const UINT32 objectCount = renderItemsCount;

		for (UINT32 frameIndex = 0; frameIndex < frameResourceCount; ++frameIndex)
		{
			const auto currentResource = frameResources[frameIndex].get();

			const auto constantBuffer = currentResource->ConstantBuffer.get();
			D3D12_GPU_VIRTUAL_ADDRESS constantBufferAddress = constantBuffer->Resource()->GetGPUVirtualAddress();

			for (UINT32 i = 0; i < objectCount; ++i)
			{
				constantBufferAddress += i * constantBufferSizeInBytes;

				const UINT32 heapIndex = frameIndex * objectCount + i;

				auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(memoryManager->GetConstantBufferViewCpu());
				handle.Offset(heapIndex, memoryManager->GetDescriptorIncrimentSize());

				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
				cbvDesc.BufferLocation = constantBufferAddress;
				cbvDesc.SizeInBytes = constantBufferSizeInBytes;
				device->CreateConstantBufferView(&cbvDesc, handle);
			}

			const UINT64 materialBufferSizeInBytes = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(MaterialConstants));

			const auto materialBuffer = currentResource->MaterialBuffer.get();
			D3D12_GPU_VIRTUAL_ADDRESS materialBufferAddress = materialBuffer->Resource()->GetGPUVirtualAddress();

			for (UINT32 i = 0; i < objectCount; ++i)
			{

				materialBufferAddress += i * materialBufferSizeInBytes;

				const UINT32 heapIndex = frameIndex * objectCount + i;

				auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(memoryManager->GetConstantBufferViewCpu());
				handle.Offset(heapIndex, memoryManager->GetDescriptorIncrimentSize());

				D3D12_CONSTANT_BUFFER_VIEW_DESC mbvDesc;
				mbvDesc.BufferLocation = materialBufferAddress;
				mbvDesc.SizeInBytes = materialBufferSizeInBytes;

				device->CreateConstantBufferView(&mbvDesc, handle);
			}

			const UINT64 passConstantBufferSizeInBytes = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(PassConstants));
			const auto passConstantBuffer = currentResource->PassBuffer->Resource();

			const D3D12_GPU_VIRTUAL_ADDRESS passConstantBufferAddress = passConstantBuffer->GetGPUVirtualAddress();

			const UINT32 heapIndex = memoryManager->GetPassBufferOffset() + frameIndex;

			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(memoryManager->GetConstantBufferViewCpu());
			handle.Offset(heapIndex, memoryManager->GetDescriptorIncrimentSize());

			D3D12_CONSTANT_BUFFER_VIEW_DESC passCbDesc;
			passCbDesc.BufferLocation = passConstantBufferAddress;
			passCbDesc.SizeInBytes = passConstantBufferSizeInBytes;

			device->CreateConstantBufferView(&passCbDesc, handle);

		}
	}


	void D3D12ResourceBuffer::UpdatePassBuffer
	(
		D3D12FrameResource* resource,
		const WorldSettings& settings,
		const MainCamera& camera, 
		const float deltaTime, 
		const float elapsedTime, 
		bool wireframe
	)
	{

		const XMMATRIX view = XMLoadFloat4x4(&camera.GetView());
		const XMMATRIX proj = XMLoadFloat4x4(&camera.GetProjection());

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
		MainPassConstantBuffer.RenderTargetSize	= XMFLOAT2((float)camera.GetBufferDimensions().x, (float)camera.GetBufferDimensions().y);
		MainPassConstantBuffer.InvRenderTargetSize = XMFLOAT2(1.0f / camera.GetBufferDimensions().x, 1.0f / camera.GetBufferDimensions().y);
		MainPassConstantBuffer.NearZ = 1.0f;
		MainPassConstantBuffer.FarZ = 1000.0f;
		MainPassConstantBuffer.AmbientLight = { settings.SunColour[0], settings.SunColour[1], settings.SunColour[2], settings.SunColour[3] };
		MainPassConstantBuffer.Lights[0].Direction = { settings.SunDirection[0], settings.SunDirection[1], settings.SunDirection[2] };
		MainPassConstantBuffer.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
		MainPassConstantBuffer.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
		MainPassConstantBuffer.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
		MainPassConstantBuffer.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
		MainPassConstantBuffer.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };
		MainPassConstantBuffer.TotalTime = elapsedTime;
		MainPassConstantBuffer.DeltaTime = deltaTime;

		const auto currentPassCB = resource->PassBuffer.get();
		currentPassCB->CopyData(0, MainPassConstantBuffer);
	}

	void D3D12ResourceBuffer::UpdateObjectBuffers(D3D12FrameResource* resource, const std::vector<RenderItem*>& renderItems)
	{
		const auto constantBuffer = resource->ConstantBuffer.get();

		for(const auto& renderItem : renderItems)
		{
			if(renderItem->NumFramesDirty > 0)
			{
				const auto d3d12RenderItem = dynamic_cast<D3D12RenderItem*>(renderItem);

				CXMMATRIX world = XMLoadFloat4x4(&d3d12RenderItem->World);
				CXMMATRIX texTransform = XMLoadFloat4x4(&d3d12RenderItem->TexTransforms);

				ObjectConstant objConst;
				XMStoreFloat4x4(&objConst.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&objConst.TexTransform, XMMatrixTranspose(texTransform));
				objConst.MaterialIndex = d3d12RenderItem->Material->GetMaterialIndex();
				objConst.ObjPad0 = 0;
				objConst.ObjPad1 = 0;
				objConst.ObjPad2 = 0;

				constantBuffer->CopyData(d3d12RenderItem->ObjectConstantBufferIndex, objConst);
				d3d12RenderItem->NumFramesDirty--;
			}
		}
	}

	void D3D12ResourceBuffer::UpdateMaterialBuffers(D3D12FrameResource* resource, const std::vector<Material*>& materials)
	{
		const auto materialBuffer = resource->MaterialBuffer.get();

		for (const auto& material : materials)
		{
			const auto d3dMaterial = dynamic_cast<D3D12Material*>(material);
			if (d3dMaterial->DirtyFrameCount > 0)
			{
				MaterialConstants matConstants;
				matConstants.DiffuseAlbedo		= d3dMaterial->DiffuseAlbedo;
				matConstants.FresnelR0			= d3dMaterial->FresnelR0;
				matConstants.Roughness			= d3dMaterial->Roughness;
				matConstants.DiffuseMapIndex	= d3dMaterial->DiffuseMapIndex;
				matConstants.NormalMapIndex		= d3dMaterial->NormalMapIndex;
				matConstants.RoughMapIndex		= d3dMaterial->RoughMapIndex;
				matConstants.AoMapIndex			= d3dMaterial->AoMapIndex;
				matConstants.HeighMapIndex		= d3dMaterial->HeightMapIndex;
				matConstants.Wire				= d3dMaterial->UseWire;

				const XMMATRIX matTransform = XMLoadFloat4x4(&d3dMaterial->MatTransform);
				XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

				materialBuffer->CopyData(d3dMaterial->MaterialBufferIndex, matConstants);
				d3dMaterial->DirtyFrameCount--;
			}
		}
	}


	const INT32 D3D12ResourceBuffer::GetCount() const
	{

		return 0;

	}

}
