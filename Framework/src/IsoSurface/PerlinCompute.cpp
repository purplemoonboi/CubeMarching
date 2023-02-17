#include "PerlinCompute.h"

#include "Framework/Maths/Perlin.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"

namespace Engine
{

	void PerlinCompute::Init(ComputeApi* context, MemoryManager* memManager, ShaderArgs args)
	{
		ComputeContext = dynamic_cast<D3D12ComputeApi*>(context);
		MemManager = dynamic_cast<D3D12MemoryManager*>(memManager);

		PerlinShader = Shader::Create(args.FilePath, args.EntryPoint, args.ShaderModel);

		BuildComputeRootSignature();
		BuildPipelineState();
		BuildResource();
		CreateReadBackBuffer();
	}

	void PerlinCompute::Dispatch(PerlinNoiseSettings args, UINT X, UINT Y, UINT Z)
	{
		ComputeContext->ResetComputeCommandList(nullptr);

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetDescriptorHeap() };
		ComputeContext->ComputeCommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);
		ComputeContext->ComputeCommandList->SetPipelineState(Pso.Get());

		auto resource = dynamic_cast<D3D12Texture*>(ScalarTexture.get());
		ComputeContext->ComputeCommandList->SetComputeRootSignature(ComputeRootSignature.Get());

		ComputeContext->ComputeCommandList->SetComputeRoot32BitConstant(0, args.Octaves, 0);
		ComputeContext->ComputeCommandList->SetComputeRoot32BitConstant(0, args.Gain, 1);
		ComputeContext->ComputeCommandList->SetComputeRoot32BitConstant(0, args.Loss, 2);
		ComputeContext->ComputeCommandList->SetComputeRoot32BitConstant(0, args.Ground, 3);
		ComputeContext->ComputeCommandList->SetComputeRootDescriptorTable(1, resource->GpuHandleUav);


		ComputeContext->ComputeCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(resource->GpuResource.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		ComputeContext->ComputeCommandList->Dispatch(X, Y, Z);

		ComputeContext->ComputeCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(resource->GpuResource.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));


		ComputeContext->ExecuteComputeCommandList();
	}

	void PerlinCompute::BuildComputeRootSignature()
	{

		CD3DX12_DESCRIPTOR_RANGE uavTable;
		uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_ROOT_PARAMETER slotRootParameter[2];
		slotRootParameter[0].InitAsConstants(4, 0); // perlin settings
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
		const auto d3d12Shader = dynamic_cast<D3D12Shader*>(PerlinShader.get());
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = ComputeRootSignature.Get();
		desc.CS =
		{
			reinterpret_cast<BYTE*>(d3d12Shader->GetShader()->GetBufferPointer()),
			d3d12Shader->GetShader()->GetBufferSize()
		};
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		const HRESULT csPipelineState = ComputeContext->Context->Device->CreateComputePipelineState
		(
			&desc,
			IID_PPV_ARGS(&Pso)
		);
		THROW_ON_FAILURE(csPipelineState);

	}

	void PerlinCompute::BuildResource()
	{

		for (INT32 i = 0; i < ChunkWidth; ++i)
			for (INT32 j = 0; j < ChunkWidth; ++j)
				for (INT32 k = 0; k < ChunkWidth; ++k)
					RawTexture.push_back(0);

		ScalarTexture = std::make_unique<D3D12Texture>(
			ChunkWidth, ChunkWidth, ChunkWidth,
			TextureDimension::Three, TextureFormat::R_FLOAT_32
			);

		ScalarTexture->InitialiseResource
		(
			RawTexture.data(),
			TextureDimension::Three,
			ComputeContext->Context,
			MemManager
		);

		
	}

	void PerlinCompute::CreateReadBackBuffer()
	{

		const auto bufferWidth = (ScalarTexture->GetWidth() * ScalarTexture->GetHeight() * ScalarTexture->GetDepth() * sizeof(float));

		const HRESULT readBackResult = ComputeContext->Context->Device->CreateCommittedResource
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

