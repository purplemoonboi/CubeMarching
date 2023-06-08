#include "Platform/DirectX12/Pipeline/D3D12PipelineResource.h"

#include "Framework/Camera/MainCamera.h"
#include "Framework/Core/Time/AppTimeManager.h"

#include "Platform/DirectX12/Materials/D3D12Material.h"
#include "Platform/DirectX12/RenderItems/D3D12RenderItem.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"


namespace Foundation::Graphics::D3D12
{
	/*
D3D12ResourceBuffer::D3D12ResourceBuffer
(
	ID3D12Device* device,
	D3D12HeapManager* memoryManager,
	const std::array<ScopePointer<D3D12FrameResource>, FRAMES_IN_FLIGHT>& frameResources,
	UINT renderItemsCount
)
{

	const auto frameResourceCount = static_cast<UINT>(frameResources.size());

	const UINT64 constantBufferSizeInBytes = D3D12BufferFactory::CalculateBufferByteSize(sizeof(ObjectConstant));

	const UINT32 objectCount = renderItemsCount;

	for (UINT32 frameIndex = 0; frameIndex < frameResourceCount; ++frameIndex)
	{
		auto currentResource = frameResources[frameIndex].get();

		const auto constantBuffer = currentResource->ConstantBuffer.get();
		D3D12_GPU_VIRTUAL_ADDRESS constantBufferAddress = constantBuffer->Resource()->GetGPUVirtualAddress();

		UINT32 i{ 0 };
		for (i = 0; i < objectCount; ++i)
		{
			constantBufferAddress += i * constantBufferSizeInBytes;

			const UINT32 heapIndex = frameIndex * objectCount + i;

			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(memoryManager->GetConstantBufferViewCpu());
			handle.Offset(heapIndex, memoryManager->GetCbvSrvUavDescSize());

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = constantBufferAddress;
			cbvDesc.SizeInBytes = constantBufferSizeInBytes;
			device->CreateConstantBufferView(&cbvDesc, handle);
		}

		const UINT64 materialBufferSizeInBytes = D3D12BufferFactory::CalculateBufferByteSize(sizeof(MaterialConstants));

		const auto materialBuffer = currentResource->MaterialBuffer.get();
		D3D12_GPU_VIRTUAL_ADDRESS materialBufferAddress = materialBuffer->Resource()->GetGPUVirtualAddress();

		for (i = 0; i < objectCount; ++i)
		{

			materialBufferAddress += i * materialBufferSizeInBytes;

			const UINT32 heapIndex = frameIndex * objectCount + i;

			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(memoryManager->GetConstantBufferViewCpu());
			handle.Offset(heapIndex, memoryManager->GetCbvSrvUavDescSize());

			D3D12_CONSTANT_BUFFER_VIEW_DESC mbvDesc;
			mbvDesc.BufferLocation = materialBufferAddress;
			mbvDesc.SizeInBytes = materialBufferSizeInBytes;

			device->CreateConstantBufferView(&mbvDesc, handle);
		}

		const UINT64 passConstantBufferSizeInBytes = D3D12BufferFactory::CalculateBufferByteSize(sizeof(PassConstants));
		const auto passConstantBuffer = currentResource->PassBuffer->Resource();

		const D3D12_GPU_VIRTUAL_ADDRESS passConstantBufferAddress = passConstantBuffer->GetGPUVirtualAddress();

		const UINT32 heapIndex = memoryManager->GetPassBufferOffset() + frameIndex;

		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(memoryManager->GetConstantBufferViewCpu());
		handle.Offset(heapIndex, memoryManager->GetCbvSrvUavDescSize());

		D3D12_CONSTANT_BUFFER_VIEW_DESC passCbDesc;
		passCbDesc.BufferLocation = passConstantBufferAddress;
		passCbDesc.SizeInBytes = passConstantBufferSizeInBytes;

		device->CreateConstantBufferView(&passCbDesc, handle);

	}
}

*/
	void D3D12ResourceBuffer::UpdatePassBuffer
	(
		D3D12FrameResource* resource,
		const MainCamera* camera,
		AppTimeManager* time,
		bool wireframe
	)
	{

		const XMMATRIX view = XMLoadFloat4x4(&camera->GetView());
		const XMMATRIX proj = XMLoadFloat4x4(&camera->GetProjection());

		const XMMATRIX viewProj = XMMatrixMultiply(view, proj);
		const XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
		const XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
		const XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

		XMStoreFloat4x4(&MainPassCB.View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&MainPassCB.InvView, XMMatrixTranspose(invView));
		XMStoreFloat4x4(&MainPassCB.Proj, XMMatrixTranspose(proj));
		XMStoreFloat4x4(&MainPassCB.InvProj, XMMatrixTranspose(invProj));
		XMStoreFloat4x4(&MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
		XMStoreFloat4x4(&MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

		MainPassCB.EyePosW = camera->GetPosition();
		MainPassCB.RenderTargetSize = XMFLOAT2((float)camera->GetBufferDimensions().x, (float)camera->GetBufferDimensions().y);
		MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / camera->GetBufferDimensions().x, 1.0f / camera->GetBufferDimensions().y);
		MainPassCB.NearZ = 1.0f;
		MainPassCB.FarZ = 1000.0f;
		MainPassCB.AmbientLight = { 1.f, 1.f, 1.f, 1.f };
		MainPassCB.Lights[0].Direction = { 0.f, -1.0f, 0.6f };
		MainPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
		MainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
		MainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
		MainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
		MainPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };
		MainPassCB.TotalTime = time->TimeElapsed();
		MainPassCB.DeltaTime = time->DeltaTime();

		const auto currentPassCB = resource->PassBuffer.get();
		currentPassCB->CopyData(0, MainPassCB);
	}

	void D3D12ResourceBuffer::UpdateVoxelTerrain
	(
		D3D12FrameResource* resource,
		RenderItem* terrain
	)
	{
		const auto voxelBuffer = resource->TerrainBuffer.get();
		const auto d3d12RenderItem = dynamic_cast<D3D12RenderItem*>(terrain);

		auto vCount = terrain->Geometry->VertexBuffer->GetCount();
		const Vertex* vertex = (const Vertex*)terrain->Geometry->VertexBuffer->GetData();

		if (vCount != 0)
		{

			{
				for (INT32 i = 0; i < vCount; ++i)
				{
					voxelBuffer->CopyData(i, vertex[i]);
				}
			}

			d3d12RenderItem->Geometry->VertexBuffer->SetBuffer(voxelBuffer->Resource());
		}
	}

	void D3D12ResourceBuffer::UpdateObjectBuffers(D3D12FrameResource* resource, const std::vector<RenderItem*>& renderItems)
	{
		const auto constantBuffer = resource->ConstantBuffer.get();

		for (const auto& renderItem : renderItems)
		{
			if (renderItem->NumFramesDirty > 0)
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
				matConstants.DiffuseAlbedo = d3dMaterial->DiffuseAlbedo;
				matConstants.FresnelR0 = d3dMaterial->FresnelR0;
				matConstants.Roughness = d3dMaterial->Roughness;
				matConstants.DiffuseMapIndex = d3dMaterial->DiffuseMapIndex;
				matConstants.NormalMapIndex = d3dMaterial->NormalMapIndex;
				matConstants.RoughMapIndex = d3dMaterial->RoughMapIndex;
				matConstants.AoMapIndex = d3dMaterial->AoMapIndex;
				matConstants.HeighMapIndex = d3dMaterial->HeightMapIndex;
				matConstants.Wire = d3dMaterial->UseWire;

				const XMMATRIX matTransform = XMLoadFloat4x4(&d3dMaterial->MatTransform);
				XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

				materialBuffer->CopyData(d3dMaterial->MaterialBufferIndex, matConstants);
				d3dMaterial->DirtyFrameCount--;
			}
		}
	}

	void D3D12ResourceBuffer::UpdateVoxelTerrainBuffer
	(
		D3D12FrameResource* resource,
		RenderItem* terrain,
		const std::vector<Vertex>& vertices
	)
	{
		const INT32 vertexCount = static_cast<INT32>(vertices.size());
		for (INT32 i = 0; i < vertexCount; ++i)
		{
			resource->TerrainBuffer->CopyData(i, vertices[i]);
		}
		//terrain->Geometry->VertexBuffer->SetData();
	}

}

