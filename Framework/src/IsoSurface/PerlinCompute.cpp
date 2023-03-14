#include "PerlinCompute.h"

#include "Framework/Maths/Perlin.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"

namespace Engine
{

	void PerlinCompute::Init(ComputeApi* context, MemoryManager* memManager)
	{
		ComputeContext = dynamic_cast<D3D12ComputeApi*>(context);
		MemManager = dynamic_cast<D3D12MemoryManager*>(memManager);

		ShaderArgs args =
		{
			L"assets\\shaders\\Perlin.hlsl",
			"ComputeNoise3D",
			"cs_5_0"
		};
		PerlinShader = Shader::Create(args.FilePath, args.EntryPoint, args.ShaderModel);

		BuildComputeRootSignature();
		BuildPipelineState();
		BuildResource();

		const HRESULT deviceRemovedReason = ComputeContext->Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);

	}

	void PerlinCompute::Dispatch(const PerlinNoiseSettings& args)
	{

		ComputeContext->ResetComputeCommandList(Pso.get());

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetShaderResourceDescHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		auto const d3d12Pso = dynamic_cast<D3D12PipelineStateObject*>(Pso.get());
		ComputeContext->CommandList->SetPipelineState(d3d12Pso->GetPipelineState());

		ComputeContext->CommandList->SetComputeRootSignature(ComputeRootSignature.Get());

		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &args.Octaves, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &args.Gain, 1);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &args.Loss, 2);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &args.GroundHeight, 3);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &args.ChunkCoord.x, 4);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &args.ChunkCoord.y, 5);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &args.ChunkCoord.z, 6);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &args.Frequency, 7);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &args.Amplitude, 8);

		auto const resource = dynamic_cast<D3D12Texture*>(ScalarTexture.get());
		ComputeContext->CommandList->SetComputeRootDescriptorTable(1, resource->GpuHandleUav);

		ComputeContext->CommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(resource->GpuResource.Get(),
				D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		ComputeContext->CommandList->Dispatch(VoxelTextureWidth, 
			VoxelTextureHeight, VoxelTextureWidth);

		ComputeContext->CommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(resource->GpuResource.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON));


		ComputeContext->FlushComputeQueue(&FenceValue);
	}

	void PerlinCompute::BuildComputeRootSignature()
	{

		CD3DX12_DESCRIPTOR_RANGE uavTable;
		uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_ROOT_PARAMETER slotRootParameter[2];
		slotRootParameter[0].InitAsConstants(10, 0); // perlin settings
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

		const HRESULT rootSigResult = ComputeContext->Context->Device->CreateRootSignature
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

		Pso = PipelineStateObject::Create(ComputeContext, PerlinShader.get(), ComputeRootSignature);
		
	}

	void PerlinCompute::BuildResource()
	{

		for (INT32 i = 0; i < VoxelTextureWidth; ++i)
			for (INT32 j = 0; j < VoxelTextureHeight; ++j)
				for (INT32 k = 0; k < VoxelTextureWidth; ++k)
					RawTexture.push_back(0);

	
		ScalarTexture = Texture::Create
		(
			RawTexture.data(),
			VoxelTextureWidth,
			VoxelTextureHeight,
			VoxelTextureWidth,
			TextureFormat::R_FLOAT_32
		);

	}


}

