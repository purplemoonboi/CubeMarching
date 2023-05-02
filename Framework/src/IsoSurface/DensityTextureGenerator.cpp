#include "DensityTextureGenerator.h"

#include "Framework/Maths/Perlin.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"

namespace Engine
{

	void DensityTextureGenerator::Init(ComputeApi* context, MemoryManager* memManager)
	{
		ComputeContext = dynamic_cast<D3D12ComputeApi*>(context);
		MemManager = dynamic_cast<D3D12MemoryManager*>(memManager);

		ShaderArgs args =
		{
			L"assets\\shaders\\DensityGenerator.hlsl",
			"ComputeNoise3D",
			"cs_5_1"
		};
		PerlinShader = Shader::Create(args.FilePath, args.EntryPoint, args.ShaderModel);

		ShaderArgs argsB =
		{
			L"assets\\shaders\\DensityGenerator.hlsl",
			"SmoothDensity",
			"cs_5_1"
		};
		SmoothShader = Shader::Create(argsB.FilePath, argsB.EntryPoint, argsB.ShaderModel);

		BuildComputeRootSignature();
		BuildPipelineState();
		BuildResource();

		const HRESULT deviceRemovedReason = ComputeContext->Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);

	}

	void DensityTextureGenerator::PerlinFBM
	(
		const PerlinNoiseSettings& noiseSettings, 
		const CSGOperationSettings& csgSettings, 
		Texture* volume
	)
	{
		if(noiseSettings.TextureWidth > volume->GetWidth() || noiseSettings.TextureHeight > volume->GetHeight())
		{
			volume->Destroy();
		}


		ComputeContext->ResetComputeCommandList(PerlinFBMPso.get());

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetShaderResourceDescHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		auto const d3d12Pso = dynamic_cast<D3D12PipelineStateObject*>(PerlinFBMPso.get());
		ComputeContext->CommandList->SetPipelineState(d3d12Pso->GetPipelineState());

		ComputeContext->CommandList->SetComputeRootSignature(DensityRootSignature.Get());

		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &noiseSettings.Octaves, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &noiseSettings.Gain, 1);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &noiseSettings.Loss, 2);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &noiseSettings.GroundHeight, 3);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &noiseSettings.ChunkCoord.x, 4);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &noiseSettings.ChunkCoord.y, 5);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &noiseSettings.ChunkCoord.z, 6);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &noiseSettings.Frequency, 7);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &noiseSettings.Amplitude, 8);

		ComputeContext->CommandList->SetComputeRoot32BitConstants(1, 1, &csgSettings.MousePos.x, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(1, 1, &csgSettings.MousePos.y, 1);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(1, 1, &csgSettings.MousePos.z, 2);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(1, 1, &csgSettings.IsMouseDown, 3);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(1, 1, &csgSettings.Radius, 4);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(1, 1, &csgSettings.DensityPrimitive, 5);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(1, 1, &csgSettings.CsgOperation, 6);


		auto const resource = dynamic_cast<D3D12Texture*>(volume);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(2, resource->GpuHandleUav);

		ComputeContext->CommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(resource->GpuResource.Get(),
				D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		ComputeContext->CommandList->Dispatch(
			noiseSettings.TextureWidth, 
			noiseSettings.TextureHeight, 
			noiseSettings.TextureWidth
		);

		ComputeContext->CommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(resource->GpuResource.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON));

		ComputeContext->FlushComputeQueue(&FenceValue);

		ResultTexture = ScalarTexture.get();
	}

	void DensityTextureGenerator::Smooth(const CSGOperationSettings& settings)
	{}

	void DensityTextureGenerator::BuildComputeRootSignature()
	{

		CD3DX12_DESCRIPTOR_RANGE primaryTexture;
		primaryTexture.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE secondaryTexture;
		secondaryTexture.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

		CD3DX12_ROOT_PARAMETER slotRootParameter[4];
		slotRootParameter[0].InitAsConstants(10, 0);				// perlin settings
		slotRootParameter[1].InitAsConstants(8, 1);					// csg settings
		slotRootParameter[2].InitAsDescriptorTable(1, &primaryTexture);		// texture
		slotRootParameter[3].InitAsDescriptorTable(1, &secondaryTexture);	// texture

		// A root signature is an array of root parameters.
		const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
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
			IID_PPV_ARGS(DensityRootSignature.GetAddressOf()
			)
		);
		THROW_ON_FAILURE(rootSigResult);
	}

	void DensityTextureGenerator::BuildPipelineState()
	{

		PerlinFBMPso = PipelineStateObject::Create(ComputeContext, PerlinShader.get(), DensityRootSignature);
		SmoothPso = PipelineStateObject::Create(ComputeContext, SmoothShader.get(), DensityRootSignature);
	}

	void DensityTextureGenerator::BuildResource()
	{

		/*for (INT32 i = 0; i < VoxelTextureWidth; ++i)
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
		);*/
	}


}

