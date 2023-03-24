#include "MarchingCubesHP.h"

#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"

#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"

#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"

namespace Engine
{
	void MarchingCubesHP::Init(ComputeApi* context, MemoryManager* memManager)
	{
		ComputeContext = dynamic_cast<D3D12ComputeApi*>(context);
		MemManager = dynamic_cast<D3D12MemoryManager*>(memManager);

		BuildRootSignature();
		BuildResources();
		BuildViews();

		/*const ShaderArgs args =
		{
			L"assets\\shaders\\Histopyramid.hlsl",
			"PrefixSum",
			"cs_5_0"
		};
		BuildUpShader = Shader::Create(args.FilePath, args.EntryPoint, args.ShaderModel);

		BuildUpPso = PipelineStateObject::Create(ComputeContext,
			BuildUpShader.get(), 
			RootSignature);

		const ShaderArgs argsB =
		{
			L"assets\\shaders\\Histopyramid.hlsl",
			"PrefixSumOffset",
			"cs_5_0"
		};
		PrefixOffsetShader = Shader::Create(argsB.FilePath, argsB.EntryPoint, argsB.ShaderModel);
		PrefixOffsetPso = PipelineStateObject::Create(ComputeContext,
			PrefixOffsetShader.get(), RootSignature);*/

		/*const ShaderArgs argsC =
		{
			L"assets\\shaders\\Histopyramid.hlsl",
			"TraverseHP",
			"cs_5_0"
		};
		StreamShader = Shader::Create(argsC.FilePath, argsC.EntryPoint, argsC.ShaderModel);
		StreamPso = PipelineStateObject::Create(ComputeContext,
			StreamShader.get(), RootSignature);*/

		const ShaderArgs argsD =
		{
			L"assets\\shaders\\Histopyramid.hlsl",
			"SortMortonCodes",
			"cs_5_0"
		};
		RadixSortShader = Shader::Create(argsD.FilePath, argsD.EntryPoint, argsD.ShaderModel);
		RadixSortPso = PipelineStateObject::Create(ComputeContext, RadixSortShader.get(), RootSignature);

		/*const ShaderArgs argsE =
		{
			L"assets\\shaders\\Histopyramid.hlsl",
			"ComputeMortonCode",
			"cs_5_0"
		};
		ComputeMortonCodes = Shader::Create(argsE.FilePath, argsE.EntryPoint, argsE.ShaderModel);
		MortonCodePso = PipelineStateObject::Create(ComputeContext, ComputeMortonCodes.get(), RootSignature);

		const ShaderArgs argsF =
		{
			L"assets\\shaders\\Histopyramid.hlsl",
			"ConstructLBVH",
			"cs_5_0"
		};
		LBVHShader = Shader::Create(argsF.FilePath, argsF.EntryPoint, argsF.ShaderModel);
		LBVHPso = PipelineStateObject::Create(ComputeContext, LBVHShader.get(), RootSignature);

		const ShaderArgs argsG =
		{
			L"assets\\shaders\\Histopyramid.hlsl",
			"PrefixSumLBVH",
			"cs_5_0"
		};
		PrefixSumLBVHShader= Shader::Create(argsG.FilePath, argsG.EntryPoint, argsG.ShaderModel);
		PrefixSumLBVHPso = PipelineStateObject::Create(ComputeContext, PrefixSumLBVHShader.get(), RootSignature);*/

		const HRESULT deviceRemovedReason = ComputeContext->Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);
	}

	void MarchingCubesHP::ConstructLBVH(Texture* texture)
	{
		ComputeContext->Wait(&FenceValue);
		ComputeContext->ResetComputeCommandList(BuildUpPso.get());

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetShaderResourceDescHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		auto const d3d12Pso = dynamic_cast<D3D12PipelineStateObject*>(BuildUpPso.get());
		ComputeContext->CommandList->SetPipelineState(d3d12Pso->GetPipelineState());

		//Bind compute shader buffers
		ComputeContext->CommandList->SetComputeRootSignature(RootSignature.Get());

		auto const d3d12Texture = dynamic_cast<D3D12Texture*>(texture);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(0, LookUpTableSrv);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(1, d3d12Texture->GpuHandleSrv);

		ComputeContext->CommandList->SetComputeRootDescriptorTable(2, TriResourceUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(3, HPResourceUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(4, MortonCodeUav);

		const UINT groupDispatch = (ChunkWidth * ChunkHeight * ChunkWidth) / 512;
		ComputeContext->CommandList->Dispatch(groupDispatch, 1, 1);

		/**
		 *		#2. Copy the morton codes back for validation
		 */
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(MortonResource.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(MortonResourceReadBack.Get(), MortonResource.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(MortonResource.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		
		/**
		 *		- Debug: copy the counter value -
		 */
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceCounter.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(CounterReadBack.Get(), ResourceCounter.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceCounter.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));



		/**
		 *		#1. copy the results from the prefix parallel sum
		 */
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutMortonResoure.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(OutMortonReadBack.Get(), OutMortonResoure.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutMortonResoure.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


	
		ComputeContext->FlushComputeQueue(&FenceValue);


		/**
		 *		#3. Map the results to local arrays
		 */
		UINT32* hp = nullptr;
		std::vector<UINT32> PrefixSum;
		PrefixSum.reserve(VoxelWorldElementCount);
		const HRESULT hr = HPResourceReadBack->Map(0, nullptr, reinterpret_cast<void**>(&hp));
		THROW_ON_FAILURE(hr);
		INT32 i;
		for(i = 0; i < VoxelWorldElementCount; ++i)
		{
			PrefixSum.push_back(hp[i]);
		}
		HPResourceReadBack->Unmap(0, nullptr);

		UINT32* mc = nullptr;
		std::vector<UINT32> MortonCodes;
		MortonCodes.reserve(VoxelWorldElementCount);
		const HRESULT hh = MortonResourceReadBack->Map(0, nullptr, reinterpret_cast<void**>(&mc));
		THROW_ON_FAILURE(hh);
		for (i = 0; i < VoxelWorldElementCount; ++i)
		{
			MortonCodes.push_back(mc[i]);
		}
		MortonResourceReadBack->Unmap(0, nullptr);

	}

	void MarchingCubesHP::StreamMCVoxels()
	{
	}

	void MarchingCubesHP::BuildRootSignature()
	{
		//	SRVs
		CD3DX12_DESCRIPTOR_RANGE lookUpTable;
		lookUpTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE textureTable;
		textureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);


		//	UAVs
		CD3DX12_DESCRIPTOR_RANGE triangleTable;
		triangleTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE histoPyramidTable;
		histoPyramidTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

		CD3DX12_DESCRIPTOR_RANGE mortonCodes;
		mortonCodes.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);

		CD3DX12_DESCRIPTOR_RANGE sortedMortons;
		sortedMortons.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);

		CD3DX12_DESCRIPTOR_RANGE bucketTable;
		bucketTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 4);

		CD3DX12_DESCRIPTOR_RANGE cycleCounter;
		cycleCounter.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 5);


		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[8];
		//  SRVs
		slotRootParameter[0].InitAsDescriptorTable(1, &lookUpTable);
		slotRootParameter[1].InitAsDescriptorTable(1, &textureTable);
		//	UAVs
		slotRootParameter[2].InitAsDescriptorTable(1, &triangleTable);					
		slotRootParameter[3].InitAsDescriptorTable(1, &histoPyramidTable);				
		slotRootParameter[4].InitAsDescriptorTable(1, &mortonCodes);
		slotRootParameter[5].InitAsDescriptorTable(1, &sortedMortons);
		slotRootParameter[6].InitAsDescriptorTable(1, &bucketTable);
		slotRootParameter[7].InitAsDescriptorTable(1, &cycleCounter);

		// A root signature is an array of root parameters.
		const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(8, slotRootParameter,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_NONE
		);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		const HRESULT hr = D3D12SerializeRootSignature
		(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}

		THROW_ON_FAILURE(hr);
		const HRESULT rootSigResult = ComputeContext->Context->Device->CreateRootSignature
		(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(RootSignature.GetAddressOf()
			)
		);
		THROW_ON_FAILURE(rootSigResult);
	}

	void MarchingCubesHP::BuildResources()
	{
		constexpr UINT64 triBufferCapacity = MarchingCubesVoxelBufferSize * sizeof(Triangle);
		TriBufferResource = D3D12BufferUtils::CreateStructuredBuffer(triBufferCapacity, true, true);
		TriReadBackResource	= D3D12BufferUtils::CreateReadBackBuffer(triBufferCapacity);

		constexpr UINT64 capacity = VoxelWorldElementCount * sizeof(INT32);
		HPResource = D3D12BufferUtils::CreateStructuredBuffer(capacity, true, true);
		HPResourceReadBack = D3D12BufferUtils::CreateReadBackBuffer(capacity);

		constexpr UINT64 mortonCapacity = VoxelWorldElementCount * sizeof(UINT32);
		MortonResource = D3D12BufferUtils::CreateStructuredBuffer(mortonCapacity, true, true);
		MortonResourceReadBack = D3D12BufferUtils::CreateReadBackBuffer(mortonCapacity);

		OutMortonResoure = D3D12BufferUtils::CreateStructuredBuffer(mortonCapacity, true, true);
		OutMortonReadBack = D3D12BufferUtils::CreateReadBackBuffer(mortonCapacity);

		constexpr UINT64 lookUpCapacity = (4096 * sizeof(INT32));
		LookUpTableResource = D3D12BufferUtils::CreateStructuredBuffer(lookUpCapacity, false, false);
		D3D12BufferUtils::CreateUploadBuffer(LookUpTableUpload, lookUpCapacity);

		ResourceCounter = D3D12BufferUtils::CreateCounterResource(true, true);
		CounterReadBack = D3D12BufferUtils::CreateReadBackBuffer(4);
		D3D12BufferUtils::CreateUploadBuffer(CounterUpload, 4);
	}

	void MarchingCubesHP::BuildViews()
	{
		/** create view for the look up table */
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.StructureByteStride = sizeof(INT32);
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = 4096;

		

		LookUpTableSrv = D3D12Utils::CreateShaderResourceView(srvDesc,
			LookUpTableResource.Get());

		
		/** create views for the vertex buffer */
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.StructureByteStride = sizeof(Triangle);
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = MarchingCubesVoxelBufferSize;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		TriResourceUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, 
			TriBufferResource.Get(), ResourceCounter.Get());

		/** create views for the histo-pyramid */
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.StructureByteStride = sizeof(INT32);
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = VoxelWorldElementCount;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		HPResourceUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, 
			HPResource.Get());

		/** create view for the morton code buffer */
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.StructureByteStride = sizeof(UINT32);
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = VoxelWorldElementCount;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		MortonCodeUav = D3D12Utils::CreateUnorderedAccessView(uavDesc,
			MortonResource.Get());

		OutMortonUav = D3D12Utils::CreateUnorderedAccessView(uavDesc,
			OutMortonResoure.Get());

		HistogramUav = D3D12Utils::CreateUnorderedAccessView(uavDesc,
			HistogramResoure.Get());
	}
}
