#include "D3D12PipelineResource.h"

#include "Framework/Camera/MainCamera.h"
#include "Framework/Core/Time/AppTimeManager.h"

#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/RenderItems/D3D12RenderItem.h"
#include "Platform/DirectX12/Resources/D3D12RenderFrame.h"


namespace Foundation::Graphics::D3D12
{
	
//D3D12ResourceBuffer::D3D12ResourceBuffer
//(
//	const std::array<ScopePointer<D3D12RenderFrame>, FRAMES_IN_FLIGHT>& frameResources,
//	UINT renderItemsCount
//)
//{
//
//	const auto frameResourceCount = static_cast<UINT>(frameResources.size());
//
//	const UINT64 constantBufferSizeInBytes = D3D12BufferFactory::CalculateBufferByteSize(sizeof(ObjectConstant));
//
//	const UINT32 objectCount = renderItemsCount;
//
//	for (UINT32 frameIndex = 0; frameIndex < frameResourceCount; ++frameIndex)
//	{
//		auto currentResource = frameResources[frameIndex].get();
//
//		const auto constantBuffer = currentResource->ConstantBuffer.get();
//		D3D12_GPU_VIRTUAL_ADDRESS constantBufferAddress = constantBuffer->Resource()->GetGPUVirtualAddress();
//
//		UINT32 i{ 0 };
//		for (i = 0; i < objectCount; ++i)
//		{
//			constantBufferAddress += i * constantBufferSizeInBytes;
//
//			const UINT32 heapIndex = frameIndex * objectCount + i;
//
//			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvHeap.BeginCPU());
//			handle.Offset(heapIndex, SrvHeap.GetSize());
//
//			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
//			cbvDesc.BufferLocation = constantBufferAddress;
//			cbvDesc.SizeInBytes = constantBufferSizeInBytes;
//			pDevice->CreateConstantBufferView(&cbvDesc, handle);
//		}
//
//		const UINT64 materialBufferSizeInBytes = D3D12BufferFactory::CalculateBufferByteSize(sizeof(MaterialConstants));
//
//		const auto materialBuffer = currentResource->MaterialBuffer.get();
//		D3D12_GPU_VIRTUAL_ADDRESS materialBufferAddress = materialBuffer->Resource()->GetGPUVirtualAddress();
//
//		for (i = 0; i < objectCount; ++i)
//		{
//
//			materialBufferAddress += i * materialBufferSizeInBytes;
//
//			const UINT32 heapIndex = frameIndex * objectCount + i;
//
//			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvHeap.BeginCPU());
//			handle.Offset(heapIndex, SrvHeap.GetSize());
//
//			D3D12_CONSTANT_BUFFER_VIEW_DESC mbvDesc;
//			mbvDesc.BufferLocation = materialBufferAddress;
//			mbvDesc.SizeInBytes = materialBufferSizeInBytes;
//
//			pDevice->CreateConstantBufferView(&mbvDesc, handle);
//		}
//
//		const UINT64 passConstantBufferSizeInBytes = D3D12BufferFactory::CalculateBufferByteSize(sizeof(PassConstants));
//		const auto passConstantBuffer = currentResource->PassBuffer->Resource();
//
//		const D3D12_GPU_VIRTUAL_ADDRESS passConstantBufferAddress = passConstantBuffer->GetGPUVirtualAddress();
//
//		const UINT32 heapIndex = memoryManager->GetPassBufferOffset() + frameIndex;
//
//		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(memoryManager->GetConstantBufferViewCpu());
//		handle.Offset(heapIndex, memoryManager->GetCbvSrvUavDescSize());
//
//		D3D12_CONSTANT_BUFFER_VIEW_DESC passCbDesc;
//		passCbDesc.BufferLocation = passConstantBufferAddress;
//		passCbDesc.SizeInBytes = passConstantBufferSizeInBytes;
//
//		device->CreateConstantBufferView(&passCbDesc, handle);
//
//	}
//}


	void D3D12ResourceBuffer::UpdatePassBuffer
	(
		const MainCamera* camera,
		AppTimeManager* time,
		bool wireframe
	)
	{

		const XMMATRIX view = XMLoadFloat4x4(&camera->GetView());
		const XMMATRIX proj = XMLoadFloat4x4(&camera->GetProjection());

		const XMMATRIX viewProj		= XMMatrixMultiply(view, proj);
		const XMMATRIX invView		= XMMatrixInverse(&XMMatrixDeterminant(view), view);
		const XMMATRIX invProj		= XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
		const XMMATRIX invViewProj	= XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

		XMStoreFloat4x4(&MainPassCB.View,			XMMatrixTranspose(view));
		XMStoreFloat4x4(&MainPassCB.InvView,		XMMatrixTranspose(invView));
		XMStoreFloat4x4(&MainPassCB.Proj,			XMMatrixTranspose(proj));
		XMStoreFloat4x4(&MainPassCB.InvProj,		XMMatrixTranspose(invProj));
		XMStoreFloat4x4(&MainPassCB.ViewProj,		XMMatrixTranspose(viewProj));
		XMStoreFloat4x4(&MainPassCB.InvViewProj,	XMMatrixTranspose(invViewProj));

		MainPassCB.EyePosW = camera->GetPosition();
		MainPassCB.RenderTargetSize = XMFLOAT2(camera->GetBufferDimensions().x, camera->GetBufferDimensions().y);
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

		const auto frame = CurrentRenderFrame();
		frame->PassBuffer.CopyData(0, MainPassCB);

	}


	void D3D12ResourceBuffer::UpdateObjectBuffers(const std::vector<RenderItem*>& renderItems)
	{
		const auto frame = CurrentRenderFrame();
		const auto constantBuffer = &frame->ConstantBuffer;

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

	void D3D12ResourceBuffer::UpdateSceneObjects(entt::registry* registry)
	{



	}
}

