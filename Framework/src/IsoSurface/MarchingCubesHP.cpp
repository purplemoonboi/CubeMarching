#include "MarchingCubesHP.h"

#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"

#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"

#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"

#include <random>

namespace Engine
{
	void MarchingCubesHP::Init(ComputeApi* context, MemoryManager* memManager)
	{
		ComputeContext = dynamic_cast<D3D12ComputeApi*>(context);
		MemManager = dynamic_cast<D3D12MemoryManager*>(memManager);

		BuildRootSignature();
		BuildResources();
		BuildViews();

		const ShaderArgs radixSort =
		{
			L"assets\\shaders\\Radix.hlsl",
			"LocalSort",
			"cs_5_0"
		};
		RadixSortShader = Shader::Create(radixSort.FilePath, radixSort.EntryPoint, radixSort.ShaderModel);
		RadixSortPso = PipelineStateObject::Create(ComputeContext, RadixSortShader.get(), RootSignature);

		const ShaderArgs globalSum =
		{
			L"assets\\shaders\\Radix.hlsl",
			"GlobalBucketSum",
			"cs_5_0"
		};
		GlobalBucketSumCS = Shader::Create(globalSum.FilePath, globalSum.EntryPoint, globalSum.ShaderModel);
		GlobalBucketSumPso = PipelineStateObject::Create(ComputeContext, GlobalBucketSumCS.get(), RootSignature);

		const ShaderArgs globalDest =
		{
			L"assets\\shaders\\Radix.hlsl",
			"GlobalDestination",
			"cs_5_0"
		};
		GlobalComputeDestCS = Shader::Create(globalSum.FilePath, globalSum.EntryPoint, globalSum.ShaderModel);
		GlobalComputeDestPso = PipelineStateObject::Create(ComputeContext, GlobalComputeDestCS.get(), RootSignature);

		const ShaderArgs genVerts =
		{
			L"assets\\shaders\\ImprovedMarchingCubes.hlsl",
			"GenerateVertices",
			"cs_5_0"
		};
		GenerateVerticesCS = Shader::Create(genVerts.FilePath, genVerts.EntryPoint, genVerts.ShaderModel);
		GenerateVerticesPso = PipelineStateObject::Create(ComputeContext, GenerateVerticesCS.get(), RootSignature);

		const ShaderArgs genTriangles =
		{
			L"assets\\shaders\\ImprovedMarchingCubes.hlsl",
			"GenerateTriangles",
			"cs_5_0"
		};
		GenerateTrianglesCS = Shader::Create(genTriangles.FilePath, genTriangles.EntryPoint, genTriangles.ShaderModel);
		GenerateTrianglesPso = PipelineStateObject::Create(ComputeContext, GenerateTrianglesCS.get(), RootSignature);


		const HRESULT deviceRemovedReason = ComputeContext->Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);
	}

	void MarchingCubesHP::RadixSort(Texture* texture)
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
		 *		#1. copy the results from the prefix parallel sum
		 */
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SortedMortonCodes.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(MortonReadBackBuffer.Get(), SortedMortonCodes.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SortedMortonCodes.Get(),
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
		const HRESULT hh = MortonReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mc));
		THROW_ON_FAILURE(hh);
		for (i = 0; i < VoxelWorldElementCount; ++i)
		{
			MortonCodes.push_back(mc[i]);
		}
		MortonReadBackBuffer->Unmap(0, nullptr);

	}

	void MarchingCubesHP::SortChunk()
	{
		constexpr UINT64 DATA_SIZE = 512;//ChunkWidth;
		ComputeContext->ResetComputeCommandList(nullptr);

		std::vector<UINT32> randMortons;
		randMortons.reserve(DATA_SIZE);

		//UINT32 keys[16] =
		//{
		//	UINT32(0 << 3 | 0 << 2 | 0 << 1 | 0), //0000
		//	UINT32(0 << 3 | 0 << 2 | 0 << 1 | 1), //0001
		//	UINT32(0 << 3 | 0 << 2 | 1 << 1 | 0), //0010
		//	UINT32(0 << 3 | 0 << 2 | 1 << 1 | 1), //0011

		//	UINT32(0 << 3 | 1 << 2 | 0 << 1 | 0), //0100
		//	UINT32(0 << 3 | 1 << 2 | 0 << 1 | 1), //0101
		//	UINT32(0 << 3 | 1 << 2 | 1 << 1 | 0), //0110
		//	UINT32(0 << 3 | 1 << 2 | 1 << 1 | 1), //0111

		//	UINT32(1 << 3 | 0 << 2 | 0 << 1 | 0), //1000
		//	UINT32(1 << 3 | 0 << 2 | 0 << 1 | 1), //1001
		//	UINT32(1 << 3 | 0 << 2 | 1 << 1 | 0), //1010
		//	UINT32(1 << 3 | 0 << 2 | 1 << 1 | 1), //1011

		//	UINT32(1 << 3 | 1 << 2 | 0 << 1 | 0), //1100
		//	UINT32(1 << 3 | 1 << 2 | 0 << 1 | 1), //1101
		//	UINT32(1 << 3 | 1 << 2 | 1 << 1 | 0), //1110
		//	UINT32(1 << 3 | 1 << 2 | 1 << 1 | 1), //1111
		//};

		for(INT32 i = 0; i < 512; ++i)
		{
			const UINT32 id = rand() % 512;
			randMortons.push_back(id);
			
		}
		

		D3D12_SUBRESOURCE_DATA srcData = {};
		srcData.pData = randMortons.data();
		srcData.RowPitch = randMortons.size() * sizeof(UINT32);
		srcData.SlicePitch = srcData.RowPitch;

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			InputMortonCodes.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
			D3D12_RESOURCE_STATE_COPY_DEST
		));

		UpdateSubresources(ComputeContext->CommandList.Get(), InputMortonCodes.Get(),
			MortonUploadBuffer.Get(), 0, 0, 1, &srcData);

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			InputMortonCodes.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		));


		ComputeContext->ExecuteComputeCommandList(&FenceValue);

		ComputeContext->Wait(&FenceValue);

		ComputeContext->ResetComputeCommandList(RadixSortPso.get());

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetShaderResourceDescHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		ComputeContext->CommandList->SetComputeRootSignature(RootSignature.Get());

		auto d3d12Pso = dynamic_cast<D3D12PipelineStateObject*>(RadixSortPso.get());
		ComputeContext->CommandList->SetPipelineState(d3d12Pso->GetPipelineState());

		ComputeContext->CommandList->SetComputeRootDescriptorTable(2, MortonCodeUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(3, SortedMortonUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(4, HistogramUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(5, CycleCounterUav);

		
		ComputeContext->CommandList->Dispatch(1, 1, 1);

		/** copy output back into input */
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			SortedMortonCodes.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE
		));

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			InputMortonCodes.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST
		));

		ComputeContext->CommandList->CopyResource(InputMortonCodes.Get(), SortedMortonCodes.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			SortedMortonCodes.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		));

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			InputMortonCodes.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		));
		

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SortedMortonCodes.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(MortonReadBackBuffer.Get(), SortedMortonCodes.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SortedMortonCodes.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


		/** counter copy */
	/*	ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			ResourceCounter.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE
		));

			ComputeContext->CommandList->CopyResource(CounterReadBack.Get(), ResourceCounter.Get());

				ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
					ResourceCounter.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS
				));

		INT32* counter = nullptr;

		const HRESULT cr = CounterReadBack->Map(0, nullptr, reinterpret_cast<void**>(&counter));
		THROW_ON_FAILURE(cr);

		INT32 ccc = *counter;
		CORE_TRACE("Number of active bits {0}", ccc);
		CounterReadBack->Unmap(0, nullptr);
		

		constexpr INT32 value[1] = { 0 };
		D3D12_SUBRESOURCE_DATA countData = {};
		countData.pData = &value[0];
		countData.RowPitch = 1 * sizeof(INT32);
		countData.SlicePitch = srcData.RowPitch;

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			ResourceCounter.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COPY_DEST
		));

		UpdateSubresources(ComputeContext->CommandList.Get(), 
			ResourceCounter.Get(), CounterUpload.Get(),
			0, 0, 1, 
			&countData
		);

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			ResourceCounter.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		));
*/


		ComputeContext->ExecuteComputeCommandList(&FenceValue);

		const HRESULT dr = ComputeContext->Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(dr);

		UINT32* data = nullptr;
		std::vector<UINT32> sortedCodes;
		sortedCodes.reserve(DATA_SIZE);

		const HRESULT hr = MortonReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&data));
		THROW_ON_FAILURE(hr);
		for (INT32 i = 0; i < DATA_SIZE; ++i)
		{
			auto val = data[i];
			sortedCodes.push_back(val);
			
		}

		MortonReadBackBuffer->Unmap(0, nullptr);
		

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


		////	UAVs
		//CD3DX12_DESCRIPTOR_RANGE triangleTable;
		//triangleTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		//CD3DX12_DESCRIPTOR_RANGE histoPyramidTable;
		//histoPyramidTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

		CD3DX12_DESCRIPTOR_RANGE mortonCodes;
		mortonCodes.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE sortedMortons;
		sortedMortons.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

		CD3DX12_DESCRIPTOR_RANGE bucketTable;
		bucketTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);

		CD3DX12_DESCRIPTOR_RANGE cycleCounter;
		cycleCounter.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);


		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[6];
		//  SRVs
		slotRootParameter[0].InitAsDescriptorTable(1, &lookUpTable);
		slotRootParameter[1].InitAsDescriptorTable(1, &textureTable);
		//	UAVs
		/*slotRootParameter[2].InitAsDescriptorTable(1, &triangleTable);					
		slotRootParameter[3].InitAsDescriptorTable(1, &histoPyramidTable);*/

		slotRootParameter[2].InitAsDescriptorTable(1, &mortonCodes);
		slotRootParameter[3].InitAsDescriptorTable(1, &sortedMortons);
		slotRootParameter[4].InitAsDescriptorTable(1, &bucketTable);
		slotRootParameter[5].InitAsDescriptorTable(1, &cycleCounter);

		// A root signature is an array of root parameters.
		const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(6, slotRootParameter,
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
		FaceBuffer = D3D12BufferUtils::CreateStructuredBuffer(triBufferCapacity, true, true);
		FaceReadBackBuffer	= D3D12BufferUtils::CreateReadBackBuffer(triBufferCapacity);

		constexpr UINT64 capacity = VoxelWorldElementCount * sizeof(INT32);
		HPResource = D3D12BufferUtils::CreateStructuredBuffer(capacity, true, true);
		HPResourceReadBack = D3D12BufferUtils::CreateReadBackBuffer(capacity);

		constexpr UINT64 mortonCapacity = VoxelWorldElementCount * sizeof(UINT32);
		InputMortonCodes = D3D12BufferUtils::CreateStructuredBuffer(mortonCapacity, true, true);
		SortedMortonCodes = D3D12BufferUtils::CreateStructuredBuffer(mortonCapacity, true, true);

		D3D12BufferUtils::CreateUploadBuffer(MortonUploadBuffer, mortonCapacity);
		MortonReadBackBuffer = D3D12BufferUtils::CreateReadBackBuffer(mortonCapacity);
		MortonReadBackBufferB = D3D12BufferUtils::CreateReadBackBuffer(mortonCapacity);



		HistogramResoure = D3D12BufferUtils::CreateStructuredBuffer(mortonCapacity, true, true);
		HistogramReadBack = D3D12BufferUtils::CreateReadBackBuffer(mortonCapacity);

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
		uavDesc.Buffer.NumElements = MarchingCubesNumberOfTriangles;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		TriResourceUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, 
			FaceBuffer.Get(), ResourceCounter.Get());

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
			InputMortonCodes.Get());

		SortedMortonUav = D3D12Utils::CreateUnorderedAccessView(uavDesc,
			SortedMortonCodes.Get());

		HistogramUav = D3D12Utils::CreateUnorderedAccessView(uavDesc,
			HistogramResoure.Get());


		/** create view for cycle counter */
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.StructureByteStride = sizeof(INT32);
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = 1;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		CycleCounterUav = D3D12Utils::CreateUnorderedAccessView(uavDesc,
			ResourceCounter.Get());
	}
}
