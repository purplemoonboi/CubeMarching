#include "D3D12RenderingApi.h"
#include "Framework/Core/Log/Log.h"
#include "Framework/ImGui/ImGuiApi/ImGuiInstructions.h"



#include "Platform/Directx12/Buffers/D3D12FrameBuffer.h"
#include "Platform/Directx12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"

#include "Platform/DirectX12/RenderItems/D3D12RenderItem.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"
#include "Platform/DirectX12/Copy/D3D12CopyContext.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"

#include "Platform/DirectX12/Textures/Loader/D3D12TextureLoader.h"

#include <imgui.h>
#include <backends/imgui_impl_dx12.h>

namespace Engine
{

#define ENGINE_IMGUI_SUPPORT 1

	D3D12RenderingApi::~D3D12RenderingApi()
	{
	}

	void D3D12RenderingApi::Init(GraphicsContext* context, INT32 viewportWidth, INT32 viewportHeight)
	{
		Context = dynamic_cast<D3D12Context*>(context);

		D3D12MemoryManager = CreateScope<class D3D12MemoryManager>(Context->Device.Get());
		const HRESULT cbvResult = D3D12MemoryManager->InitialiseCbvHeap(Context->Device.Get(), NUMBER_OF_FRAME_RESOURCES, 12);
		THROW_ON_FAILURE(cbvResult);
		
		const HRESULT srvUavResult = D3D12MemoryManager->InitialiseSrvUavHeap(Context->Device.Get(), 12);
		THROW_ON_FAILURE(srvUavResult);

		D3D12TextureLoader::Init(Context);
		D3D12BufferUtils::Init(Context->Device.Get(), Context->GraphicsCmdList.Get());
		D3D12Utils::Init(D3D12MemoryManager.get(), Context);

		D3D12CopyContext::Init(Context);

		FrameBufferSpecifications fbs;
		fbs.Width = viewportWidth;
		fbs.Height = viewportHeight;
		

		FrameBuffer = std::make_unique<class D3D12FrameBuffer>(fbs);
		FrameBuffer->Init(Context);
		FrameBuffer->RebuildFrameBuffer(fbs);

		std::vector<INT8> bytes;
		bytes.insert(bytes.begin(), 1920 * 1080, 0);
		RenderTarget = CreateScope<D3D12RenderTarget>(bytes.data(), 1920, 1080);

		constexpr UINT32 maxObjCount = 16;
		constexpr UINT32 maxMatCount = 16;

		for (int i = 0; i < NUMBER_OF_FRAME_RESOURCES; ++i)
		{
			FrameResources.push_back(CreateScope<D3D12FrameResource>
			(
				Context,
				1,
				maxMatCount,
				maxObjCount
			)
			);
		}

		UploadBuffer = CreateScope<D3D12ResourceBuffer>
		(
			Context->Device.Get(),
			D3D12MemoryManager.get(),
			FrameResources,
			2
		);

	}



	void D3D12RenderingApi::SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		if(Context != nullptr && FrameBuffer != nullptr)
		{
			CORE_TRACE("Buffer resize");
			FrameBufferSpecifications fbSpecs = {};
			fbSpecs.Width = width;
			fbSpecs.Height = height;
			fbSpecs.OffsetX = x;
			fbSpecs.OffsetY = y;
			FrameBuffer->RebuildFrameBuffer(fbSpecs);
		}
	}

	void D3D12RenderingApi::BindDepthPass()
	{
	}

	

	void D3D12RenderingApi::PreInit()
	{
		// Reset the command allocator
		// Reset the command list
		THROW_ON_FAILURE(Context->GraphicsCmdList->Reset
		(
			Context->Allocator.Get(),
			nullptr
		));
	}


	void D3D12RenderingApi::PostInit()
	{
		// Execute the initialization commands.
		HRESULT closureResult = Context->GraphicsCmdList->Close();
		THROW_ON_FAILURE(closureResult);
		ID3D12CommandList* cmdsLists[] = { Context->GraphicsCmdList.Get() };
		Context->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		Context->FlushCommandQueue();
	}

	void D3D12RenderingApi::PreRender(
		const std::vector<RenderItem*>& items, const std::vector<Material*>& materials,
		const WorldSettings& settings,
		const MainCamera& camera,
		float deltaTime,
		float elapsedTime,
		bool wireframe
	)
	{

		CurrentFrameResourceIndex = (CurrentFrameResourceIndex + 1) % NUMBER_OF_FRAME_RESOURCES;
		CurrentFrameResource = FrameResources[CurrentFrameResourceIndex].get();
		CORE_ASSERT(CurrentFrameResource, "No valid frame resource!");

		// Has the GPU finished processing the commands of the current frame resource
		// If not, wait until the GPU has completed commands up to this fence point.
		auto val = Context->Fence->GetCompletedValue();
		if (val < CurrentFrameResource->SignalCount)
		{
			const HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			const HRESULT eventCompletion = Context->Fence->SetEventOnCompletion(CurrentFrameResource->SignalCount, eventHandle);
			THROW_ON_FAILURE(eventCompletion);
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}

		// Update constant buffers for each render item and material
		UploadBuffer->UpdateObjectBuffers(CurrentFrameResource, items);
		UploadBuffer->UpdateMaterialBuffers(CurrentFrameResource, materials);
		UploadBuffer->UpdatePassBuffer(CurrentFrameResource, settings, camera, deltaTime, elapsedTime, wireframe);

		// If everything checks out prepare recording instructions under the
		// current frame resource.
		CORE_ASSERT(Context->Device, "Device lost");
		CORE_ASSERT(Context->GraphicsCmdList, "Graphics CmdL lost");
		CORE_ASSERT(Context->CommandQueue, "Command queue lost");

		// Check for any scheduled buffer regenerations
		const HRESULT cmdAllocReset = CurrentFrameResource->CmdListAlloc->Reset();
		THROW_ON_FAILURE(cmdAllocReset);
		const HRESULT cmdReset = Context->GraphicsCmdList->Reset(CurrentFrameResource->CmdListAlloc.Get(), nullptr);
		THROW_ON_FAILURE(cmdReset);

		for(auto item : items)
		{
			if(item->Geometry->DirtFlag == 1)
			{
				auto* vb = dynamic_cast<D3D12VertexBuffer*>(item->Geometry->VertexBuffer.get());
				vb->Regenerate();
				auto* ib = dynamic_cast<D3D12IndexBuffer*>(item->Geometry->IndexBuffer.get());
				ib->Regenerate();
				item->Geometry->DirtFlag = 0;
			}
		}

		const HRESULT closeResult = Context->GraphicsCmdList->Close();
		THROW_ON_FAILURE(closeResult);
		ID3D12CommandList* cmdsLists[] = { Context->GraphicsCmdList.Get() };
		Context->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		CurrentFrameResource->SignalCount = ++Context->SyncCounter;

		const HRESULT signalResult = Context->CommandQueue->Signal(Context->Fence.Get(), CurrentFrameResource->SignalCount);
		THROW_ON_FAILURE(signalResult);

		if(Context->Fence->GetCompletedValue() < CurrentFrameResource->SignalCount)
		{
			const HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			const HRESULT eventCompletion = Context->Fence->SetEventOnCompletion(CurrentFrameResource->SignalCount, eventHandle);
			THROW_ON_FAILURE(eventCompletion);
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
		
		const HRESULT cmdResetBeginRender = CurrentFrameResource->CmdListAlloc->Reset();
		THROW_ON_FAILURE(cmdResetBeginRender);
		const HRESULT cmdListBeginRender = Context->GraphicsCmdList->Reset(CurrentFrameResource->CmdListAlloc.Get(), nullptr);
		THROW_ON_FAILURE(cmdListBeginRender);


	}

	void D3D12RenderingApi::PostRender()
	{



	}

	void D3D12RenderingApi::BindGeometryPass(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems)
	{
		//RenderTarget->Bind(Context);


		Context->GraphicsCmdList->RSSetViewports(1, &RenderTarget->Viewport);
		Context->GraphicsCmdList->RSSetScissorRects(1, &RenderTarget->Rect);

		// Change offscreen texture to be used as a a render target output.
		Context->GraphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(RenderTarget->GpuResource.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

		// Clear the back buffer and depth buffer.
		Context->GraphicsCmdList->ClearRenderTargetView(RenderTarget->ResourceCpuRtv, DirectX::Colors::SandyBrown, 0, nullptr);
		Context->GraphicsCmdList->ClearDepthStencilView(FrameBuffer->GetDepthStencilViewCpu(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// Specify the buffers we are going to render to.
		Context->GraphicsCmdList->OMSetRenderTargets(1, &RenderTarget->ResourceCpuRtv, 
			true, &FrameBuffer->GetDepthStencilViewCpu());



		const auto dx12Pso = dynamic_cast<D3D12PipelineStateObject*>(pso);
		Context->GraphicsCmdList->SetPipelineState(dx12Pso->GetPipelineState());
		
		ID3D12DescriptorHeap* descriptorHeaps[] = { D3D12MemoryManager->GetConstantBufferDescHeap() };
		Context->GraphicsCmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		/* Bind the shader root signature */
		Context->GraphicsCmdList->SetGraphicsRootSignature(Context->RootSignature.Get());

		const D3D12_GPU_VIRTUAL_ADDRESS passBufferAddress = CurrentFrameResource->PassBuffer->Resource()->GetGPUVirtualAddress();
		Context->GraphicsCmdList->SetGraphicsRootConstantBufferView(3, passBufferAddress);

		// For each render item...
		for (auto& renderItem : renderItems)
		{
			const auto renderItemDerived = dynamic_cast<D3D12RenderItem*>(renderItem);
			const auto d3d12VertexBuffer = dynamic_cast<D3D12VertexBuffer*>(renderItem->Geometry->VertexBuffer.get());
			const auto d3d12IndexBuffer = dynamic_cast<D3D12IndexBuffer*>(renderItem->Geometry->IndexBuffer.get());

			Context->GraphicsCmdList->IASetVertexBuffers(0, 1, &d3d12VertexBuffer->GetVertexBufferView());
			Context->GraphicsCmdList->IASetIndexBuffer(&d3d12IndexBuffer->GetIndexBufferView());
			Context->GraphicsCmdList->IASetPrimitiveTopology(renderItemDerived->PrimitiveType);

			const UINT objConstBufferByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));
			const UINT matConstBufferByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(MaterialConstants));

			ID3D12Resource* objectConstantBuffer = CurrentFrameResource->ConstantBuffer->Resource();
			ID3D12Resource* materialConstantBuffer = CurrentFrameResource->MaterialBuffer->Resource();

			const D3D12_GPU_VIRTUAL_ADDRESS objConstBufferAddress = objectConstantBuffer->GetGPUVirtualAddress() + renderItem->ObjectConstantBufferIndex * objConstBufferByteSize;
			const D3D12_GPU_VIRTUAL_ADDRESS materialBufferAddress = materialConstantBuffer->GetGPUVirtualAddress() + renderItem->Material->GetBufferIndex() * matConstBufferByteSize;

			//if(renderItem->Texture != nullptr)
			//{
			//	const auto d3dTexture = dynamic_cast<D3D12Texture*>(renderItem->Texture);

			//	/*CD3DX12_GPU_DESCRIPTOR_HANDLE texture(D3D12MemoryManager->GetShaderResourceDescHeap()->GetGPUDescriptorHandleForHeapStart());
			//	texture.Offset(renderItem->Material->GetBufferIndex(), D3D12MemoryManager->GetDescriptorIncrimentSize());
			//	Context->GraphicsCmdList->SetGraphicsRootDescriptorTable(0, texture);*/

			//	Context->GraphicsCmdList->SetGraphicsRootDescriptorTable(0, d3dTexture->GpuHandleSrv);
			//}
			
			
			Context->GraphicsCmdList->SetGraphicsRootConstantBufferView(1, objConstBufferAddress);
			Context->GraphicsCmdList->SetGraphicsRootConstantBufferView(2, materialBufferAddress);
			
			if (renderItem->Geometry->GetName() == "MarchingTerrain" || renderItem->Geometry->GetName() == "DualTerrain")
			{
				Context->GraphicsCmdList->DrawInstanced
				(
					renderItem->Geometry->VertexBuffer->GetCount(),
					1,
					renderItem->StartIndexLocation,
					renderItem->BaseVertexLocation
				);
			}
			else
			{
				Context->GraphicsCmdList->DrawIndexedInstanced
				(
					renderItem->Geometry->IndexBuffer->GetCount(),
					1,
					renderItem->StartIndexLocation,
					renderItem->BaseVertexLocation,
					0
				);
			}

		}

		// Change offscreen texture to be used as a a render target output.
		Context->GraphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(RenderTarget->GpuResource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));


		//RenderTarget->UnBind(Context);
	}

	void D3D12RenderingApi::BindTerrainPass
	(
		PipelineStateObject* pso, 
		const MeshGeometry* terrainMesh,
		UINT constantBufferOffset, UINT materialBufferOffset
	
	)
	{
		const auto dx12Pso = dynamic_cast<D3D12PipelineStateObject*>(pso);
		Context->GraphicsCmdList->SetPipelineState(dx12Pso->GetPipelineState());
		Context->GraphicsCmdList->RSSetViewports(1, &FrameBuffer->GetViewport());
		Context->GraphicsCmdList->RSSetScissorRects(1, &FrameBuffer->GetScissorsRect());



		Context->GraphicsCmdList->OMSetRenderTargets(1, &FrameBuffer->GetCurrentBackBufferViewCpu(),
			true,
			&FrameBuffer->GetDepthStencilViewCpu()
		);
		ID3D12DescriptorHeap* descriptorHeaps[] = { D3D12MemoryManager->GetConstantBufferDescHeap() };
		Context->GraphicsCmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		/* Bind the shader root signature */
		Context->GraphicsCmdList->SetGraphicsRootSignature(Context->RootSignature.Get());
		const D3D12_GPU_VIRTUAL_ADDRESS passBufferAddress = CurrentFrameResource->PassBuffer->Resource()->GetGPUVirtualAddress();
		Context->GraphicsCmdList->SetGraphicsRootConstantBufferView(2, passBufferAddress);
	
		const auto d3d12VertexBuffer = dynamic_cast<D3D12VertexBuffer*>(terrainMesh->VertexBuffer.get());
		const auto d3d12IndexBuffer = dynamic_cast<D3D12IndexBuffer*>(terrainMesh->IndexBuffer.get());

		Context->GraphicsCmdList->IASetVertexBuffers(0, 1, &d3d12VertexBuffer->GetVertexBufferView());
		Context->GraphicsCmdList->IASetIndexBuffer(&d3d12IndexBuffer->GetIndexBufferView());
		Context->GraphicsCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		const UINT objConstBufferByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));
		const UINT matConstBufferByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(MaterialConstants));

		ID3D12Resource* objectConstantBuffer = CurrentFrameResource->ConstantBuffer->Resource();
		ID3D12Resource* materialConstantBuffer = CurrentFrameResource->MaterialBuffer->Resource();

		const D3D12_GPU_VIRTUAL_ADDRESS objConstBufferAddress = objectConstantBuffer->GetGPUVirtualAddress() + constantBufferOffset * objConstBufferByteSize;
		const D3D12_GPU_VIRTUAL_ADDRESS materialBufferAddress = materialConstantBuffer->GetGPUVirtualAddress() + materialBufferOffset * matConstBufferByteSize;

		Context->GraphicsCmdList->SetGraphicsRootConstantBufferView(0, objConstBufferAddress);
		Context->GraphicsCmdList->SetGraphicsRootConstantBufferView(1, materialBufferAddress);


		Context->GraphicsCmdList->DrawInstanced
		(
			terrainMesh->VertexBuffer->GetCount(),
			1,
			terrainMesh->DrawArgs.at("Terrain").StartIndexLocation,
			terrainMesh->DrawArgs.at("Terrain").BaseVertexLocation
		);

		
	}

	void D3D12RenderingApi::BindLightingPass()
	{

	}

	void D3D12RenderingApi::BindPostProcessingPass()
	{
		
	}

	void D3D12RenderingApi::Flush()
	{

		
		/**
		 * Deffer presenting until we have recorded the commands for ImGui
		 * If ImGui is supported
		 */

		//const HRESULT closeResult = Context->GraphicsCmdList->Close();
		//THROW_ON_FAILURE(closeResult);

		//ID3D12CommandList* cmdList[] = { Context->GraphicsCmdList.Get() };
		//Context->CommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

		//// Has the GPU finished processing the commands of the current frame resource
		//// If not, wait until the GPU has completed commands up to this fence point.
		//if (Context->Fence->GetCompletedValue() < CurrentFrameResource->SignalCount)
		//{
		//	const HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		//	const HRESULT eventCompletion = Context->Fence->SetEventOnCompletion(CurrentFrameResource->SignalCount, eventHandle);
		//	THROW_ON_FAILURE(eventCompletion);
		//	WaitForSingleObject(eventHandle, INFINITE);
		//	CloseHandle(eventHandle);
		//}

		//const HRESULT cr = CurrentFrameResource->CmdListAlloc->Reset();
		//THROW_ON_FAILURE(cr);

		//const HRESULT hr = Context->GraphicsCmdList->Reset(CurrentFrameResource->CmdListAlloc.Get(), nullptr);
		//THROW_ON_FAILURE(hr);

#ifndef ENGINE_IMGUI_SUPPORT
		FrameBuffer->UnBind();


		
		const HRESULT presentResult = Context->SwapChain->Present(0, 0);
		THROW_ON_FAILURE(presentResult);
		FrameBuffer->SetBackBufferIndex((FrameBuffer->GetBackBufferIndex() + 1) % SWAP_CHAIN_BUFFER_COUNT);
		CurrentFrameResource->Fence = ++Context->SyncCounter;

		/**
		 * Add an instruction to the command queue to set a new fence point. 
		 * Because we are on the GPU timeline, the new fence point won't be 
		 * set until the GPU finishes processing all the commands prior to this Signal().
		 */
		const HRESULT signalResult = Context->CommandQueue->Signal(Context->Fence.GetShader(), Context->SyncCounter);
		THROW_ON_FAILURE(signalResult);

#endif
	}

}
