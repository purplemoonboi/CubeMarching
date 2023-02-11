#include "PerlinCompute.h"

#include "Framework/Maths/Perlin.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"

namespace Engine
{

	void PerlinCompute::Init(GraphicsContext* context, MemoryManager* memManager)
	{
		Context = dynamic_cast<D3D12Context*>(context);
		MemManager = dynamic_cast<D3D12MemoryManager*>(memManager);

		BuildComputeRootSignature();
		BuildPipelineState();
		BuildResource();
		CreateReadBackBuffer();
	}

	void PerlinCompute::Generate3DTexture(PerlinArgs args, UINT X, UINT Y, UINT Z)
	{
		const HRESULT cmdAllocResult = Context->CmdListAlloc->Reset();
		THROW_ON_FAILURE(cmdAllocResult);

		const HRESULT cmdGraphicsResult = Context->GraphicsCmdList->Reset(Context->CmdListAlloc.Get(), Pso.Get());
		THROW_ON_FAILURE(cmdGraphicsResult);

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetDescriptorHeap() };
		Context->GraphicsCmdList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);
		Context->GraphicsCmdList->SetPipelineState(Pso.Get());

		auto resource = dynamic_cast<D3D12Texture*>(ScalarTexture.get());
		Context->GraphicsCmdList->SetComputeRootSignature(ComputeRootSignature.Get());

		Context->GraphicsCmdList->SetComputeRoot32BitConstant(0, args.Octaves, 0);
		Context->GraphicsCmdList->SetComputeRoot32BitConstant(0, args.Gain, 1);
		Context->GraphicsCmdList->SetComputeRoot32BitConstant(0, args.Loss, 2);
		Context->GraphicsCmdList->SetComputeRootDescriptorTable(1, resource->GpuHandleUav);


		Context->GraphicsCmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(resource->GpuResource.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		Context->GraphicsCmdList->Dispatch(X, Y, Z);

		Context->GraphicsCmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(resource->GpuResource.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));


		// Execute the commands and flush
		THROW_ON_FAILURE(Context->GraphicsCmdList->Close());
		ID3D12CommandList* cmdsLists[] = { Context->GraphicsCmdList.Get() };
		Context->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		Context->FlushCommandQueue();


		int i = 0;
	}

	void PerlinCompute::BuildComputeRootSignature()
	{

		CD3DX12_DESCRIPTOR_RANGE uavTable;
		uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_ROOT_PARAMETER slotRootParameter[2];
		slotRootParameter[0].InitAsConstants(3, 0); // perlin settings
		slotRootParameter[1].InitAsDescriptorTable(1, &uavTable);// texture

		// A root signature is an array of root parameters.
		const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_NONE
		);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		const HRESULT serialisedResult = D3D12SerializeRootSignature
		(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (errorBlob != nullptr) { ::OutputDebugStringA((char*)errorBlob->GetBufferPointer()); }

		THROW_ON_FAILURE(serialisedResult);
		const HRESULT rootSigResult = Context->Device->CreateRootSignature
		(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(ComputeRootSignature.GetAddressOf()
			)
		);
		THROW_ON_FAILURE(rootSigResult);
	}

	void PerlinCompute::BuildPipelineState()
	{
		const auto d3d12Shader = dynamic_cast<D3D12Shader*>(PerlinShader.get());
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = ComputeRootSignature.Get();
		desc.CS =
		{
			reinterpret_cast<BYTE*>(d3d12Shader->GetShader()->GetBufferPointer()),
			d3d12Shader->GetShader()->GetBufferSize()
		};
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		const HRESULT csPipelineState = Context->Device->CreateComputePipelineState
		(
			&desc,
			IID_PPV_ARGS(&Pso)
		);
		THROW_ON_FAILURE(csPipelineState);

	}

	void PerlinCompute::BuildResource()
	{

		for (INT32 i = 0; i < VoxelWorldSize; ++i)
			for (INT32 j = 0; j < VoxelWorldSize; ++j)
				for (INT32 k = 0; k < VoxelWorldSize; ++k)
					RawTexture.push_back(0);

		ScalarTexture = std::make_unique<D3D12Texture>(
			VoxelWorldSize, VoxelWorldSize, VoxelWorldSize,
			TextureDimension::Three, TextureFormat::R_FLOAT_32
			);

		ScalarTexture->InitialiseResource
		(
			RawTexture.data(),
			TextureDimension::Three,
			Context,
			MemManager
		);

		
	}

	void PerlinCompute::CreateReadBackBuffer()
	{

		const auto bufferWidth = (ScalarTexture->GetWidth() * ScalarTexture->GetHeight() * ScalarTexture->GetDepth() * sizeof(float));

		const HRESULT readBackResult = Context->Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&ReadBackBuffer)
		);

		THROW_ON_FAILURE(readBackResult);

	}
}

