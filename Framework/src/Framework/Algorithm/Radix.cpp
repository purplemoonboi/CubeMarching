#include "Radix.h"
#include "Framework/Renderer/Resources/Shader.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"

namespace Engine
{
	void Radix::Init(ComputeApi* context, MemoryManager* memManager)
	{
		ComputeContext = dynamic_cast<D3D12ComputeApi*>(context);
		MemManager = dynamic_cast<D3D12HeapManager*>(memManager);

		BuildRootSignature();
		BuildResources();
		BuildViews();

		const ShaderArgs computeMorton =
		{
			L"assets\\shaders\\Radix.hlsl",
			"EncodePoint",
			"cs_5_0"
		};
		ComputeMortonCS = Shader::Create(computeMorton.FilePath, computeMorton.EntryPoint, computeMorton.ShaderModel);
		ComputeMortonPso = PipelineStateObject::Create(ComputeContext, ComputeMortonCS.get(), RootSignature);


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
	}


	void Radix::SortChunk(const VoxelWorldSettings& settings)
	{

		const UINT32 dispatchX = VoxelWorldElementCount / 512;

		ComputeContext->ResetComputeCommandList(ComputeMortonPso.get());
		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetShaderResourceDescHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		ComputeContext->CommandList->SetComputeRootSignature(RootSignature.Get());

		ComputeContext->CommandList->SetComputeRootDescriptorTable(0, MortonCodeUav);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(4, 1, &settings.Resolution, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(4, 1, &settings.Resolution, 1);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(4, 1, &settings.Resolution, 2);

		ComputeContext->CommandList->Dispatch(dispatchX, 1, 1);

		INT32 value[1] = { 0 };
		D3D12_SUBRESOURCE_DATA countData = {};
		countData.pData = &value[0];
		countData.RowPitch = 1 * sizeof(INT32);
		countData.SlicePitch = countData.RowPitch;

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CycleCounter.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COPY_DEST
		));

		UpdateSubresources(ComputeContext->CommandList.Get(),
			CycleCounter.Get(), CycleCounterUpload.Get(),
			0, 0, 1,
			&countData
		);

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CycleCounter.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		));



		ComputeContext->FlushComputeQueue(&FenceValue);


		ComputeContext->ResetComputeCommandList(RadixSortPso.get());

		srvHeap[0] = { MemManager->GetShaderResourceDescHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		ComputeContext->CommandList->SetComputeRootSignature(RootSignature.Get());



		for(INT32 i = 0; i < 1; ++i)
		{
			auto d3d12Pso = dynamic_cast<D3D12PipelineStateObject*>(RadixSortPso.get());
			ComputeContext->CommandList->SetPipelineState(d3d12Pso->GetPipelineState());

			ComputeContext->CommandList->SetComputeRootDescriptorTable(0, MortonCodeUav);
			ComputeContext->CommandList->SetComputeRootDescriptorTable(1, SortedMortonUav);
			ComputeContext->CommandList->SetComputeRootDescriptorTable(2, GlobalBucketsUav);
			ComputeContext->CommandList->SetComputeRootDescriptorTable(3, CycleCounterUav);
			ComputeContext->CommandList->SetComputeRoot32BitConstants(4, 1, &settings.Resolution, 0);
			ComputeContext->CommandList->SetComputeRoot32BitConstants(4, 1, &settings.Resolution, 1);
			ComputeContext->CommandList->SetComputeRoot32BitConstants(4, 1, &settings.Resolution, 2);

			/*..Dispatch Local Sort..*/
			ComputeContext->CommandList->Dispatch(dispatchX, 1, 1);

			/*..Dispatch Global Sum..*/
			d3d12Pso = dynamic_cast<D3D12PipelineStateObject*>(GlobalBucketSumPso.get());
			ComputeContext->CommandList->SetPipelineState(d3d12Pso->GetPipelineState());

			ComputeContext->CommandList->SetComputeRootDescriptorTable(0, MortonCodeUav);
			ComputeContext->CommandList->SetComputeRootDescriptorTable(1, SortedMortonUav);
			ComputeContext->CommandList->SetComputeRootDescriptorTable(2, GlobalBucketsUav);
			ComputeContext->CommandList->SetComputeRootDescriptorTable(3, CycleCounterUav);

			ComputeContext->CommandList->Dispatch(1, 1, 1);

			/*..Dispatch Global scatter..*/
			d3d12Pso = dynamic_cast<D3D12PipelineStateObject*>(GlobalComputeDestPso.get());
			ComputeContext->CommandList->SetPipelineState(d3d12Pso->GetPipelineState());

			ComputeContext->CommandList->SetComputeRootDescriptorTable(0, MortonCodeUav);
			ComputeContext->CommandList->SetComputeRootDescriptorTable(1, SortedMortonUav);
			ComputeContext->CommandList->SetComputeRootDescriptorTable(2, GlobalBucketsUav);
			ComputeContext->CommandList->SetComputeRootDescriptorTable(3, CycleCounterUav);

			ComputeContext->CommandList->Dispatch(dispatchX, 1, 1);

		}
		
		ComputeContext->FlushComputeQueue(&FenceValue);
		ComputeContext->ResetComputeCommandList(nullptr);

		/** copy output back into input */
		/*ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
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
		));*/


		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(InputMortonCodes.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(MortonReadBackBuffer.Get(), InputMortonCodes.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(InputMortonCodes.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


		/** counter copy */
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CycleCounter.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE
		));

			ComputeContext->CommandList->CopyResource(CycleCounterReadBack.Get(), CycleCounter.Get());

				ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
					CycleCounter.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS
				));

		INT32* counter = nullptr;

		const HRESULT cr = CycleCounterReadBack->Map(0, nullptr, reinterpret_cast<void**>(&counter));
		THROW_ON_FAILURE(cr);

		INT32 ccc = *counter;
		CORE_TRACE("Current cycle {0}", ccc);
		CycleCounter->Unmap(0, nullptr);


		value[0] = { 0 };
		countData = {};
		countData.pData = &value[0];
		countData.RowPitch = 1 * sizeof(INT32);
		countData.SlicePitch = countData.RowPitch;

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CycleCounter.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COPY_DEST
		));

		UpdateSubresources(ComputeContext->CommandList.Get(),
			CycleCounter.Get(), CycleCounterUpload.Get(),
			0, 0, 1,
			&countData
		);

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CycleCounter.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		));



		ComputeContext->FlushComputeQueue(&FenceValue);

		const HRESULT dr = ComputeContext->Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(dr);

		UINT32* data = nullptr;
		std::vector<UINT32> sortedCodes;
		sortedCodes.reserve(VoxelWorldElementCount);

		const HRESULT hr = MortonReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&data));
		THROW_ON_FAILURE(hr);
		for (INT32 i = 0; i < VoxelWorldElementCount; ++i)
		{
			auto val = data[i];
			sortedCodes.push_back(val);

		}

		MortonReadBackBuffer->Unmap(0, nullptr);


	}

	void Radix::BuildResources()
	{
		constexpr UINT64 mortonCapacity = VoxelWorldElementCount * sizeof(UINT32);
		InputMortonCodes = D3D12BufferUtilities::CreateStructuredBuffer(mortonCapacity, true, true);
		SortedMortonCodes = D3D12BufferUtilities::CreateStructuredBuffer(mortonCapacity, true, true);

		MortonReadBackBuffer = D3D12BufferUtilities::CreateReadBackBuffer(mortonCapacity);
		MortonReadBackBufferB = D3D12BufferUtilities::CreateReadBackBuffer(mortonCapacity);
		D3D12BufferUtilities::CreateUploadBuffer(MortonUploadBuffer, mortonCapacity);

		constexpr UINT64 bucketsCapacity = (VoxelWorldElementCount) * sizeof(INT32);
		GlobalBuckets = D3D12BufferUtilities::CreateStructuredBuffer(bucketsCapacity, true, true);
		GlobalBucketsReadBack = D3D12BufferUtilities::CreateReadBackBuffer(bucketsCapacity);
		D3D12BufferUtilities::CreateUploadBuffer(GlobalBucketsUpload, bucketsCapacity);

		constexpr UINT64 capacity = sizeof(INT32);
		CycleCounter = D3D12BufferUtilities::CreateStructuredBuffer(capacity,true, true);
		CycleCounterReadBack = D3D12BufferUtilities::CreateReadBackBuffer(capacity);
		D3D12BufferUtilities::CreateUploadBuffer(CycleCounterUpload, capacity);
	}


	void Radix::BuildViews()
	{
		/** create views for the histo-pyramid */

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
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

		uavDesc.Buffer.NumElements = (VoxelWorldElementCount / 512);
		GlobalBucketsUav = D3D12Utils::CreateUnorderedAccessView(uavDesc,
			GlobalBuckets.Get());



		/** we only need one element for the counter buffer */
		uavDesc.Buffer.StructureByteStride = sizeof(INT32);
		uavDesc.Buffer.NumElements = 1;
		CycleCounterUav = D3D12Utils::CreateUnorderedAccessView(uavDesc,
			CycleCounter.Get());

	}

	void Radix::BuildRootSignature()
	{


		CD3DX12_DESCRIPTOR_RANGE mortonCodes;
		mortonCodes.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE sortedMortons;
		sortedMortons.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

		CD3DX12_DESCRIPTOR_RANGE bucketTable;
		bucketTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);

		CD3DX12_DESCRIPTOR_RANGE cycleCounter;
		cycleCounter.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);


		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[5];
		slotRootParameter[0].InitAsDescriptorTable(1, &mortonCodes);
		slotRootParameter[1].InitAsDescriptorTable(1, &sortedMortons);
		slotRootParameter[2].InitAsDescriptorTable(1, &bucketTable);
		slotRootParameter[3].InitAsDescriptorTable(1, &cycleCounter);
		slotRootParameter[4].InitAsConstants(3, 0);

		// A root signature is an array of root parameters.
		const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter,
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


}

