#include "DualContouring.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"

namespace Engine
{
	void DualContouring::Init(ComputeApi* compute, MemoryManager* memManger, ShaderArgs& args)
	{
		ComputeContext = dynamic_cast<D3D12ComputeApi*>(compute);
		MemManager = dynamic_cast<D3D12MemoryManager*>(memManger);

		DualShader = Shader::Create(args.FilePath, args.EntryPoint, args.ShaderModel);

		BuildRootSignature();
		BuildDualContourPipelineState();
		CreateBufferCounter();
		CreateOutputBuffer();

	}

	void DualContouring::Dispatch(VoxelWorldSettings& settings, Texture* texture, INT32 X, INT32 Y, INT32 Z)
	{
		ComputeContext->ResetComputeCommandList(DualPipelineStateObj.get());

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetDescriptorHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		auto const d3d12Pso = dynamic_cast<D3D12PipelineStateObject*>(DualPipelineStateObj.get());
		ComputeContext->CommandList->SetPipelineState(d3d12Pso->GetPipelineState());

		//Bind compute shader buffers
		ComputeContext->CommandList->SetComputeRootSignature(RootSignature.Get());

		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.IsoValue, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.TextureSize, 1);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.PlanetRadius, 2);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.NumOfPointsPerAxis, 3);

		const float coord[3] = { settings.ChunkCoord.x, settings.ChunkCoord.y, settings.ChunkCoord.z };
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 3, coord, 4);

		auto const d3d12Texture = dynamic_cast<D3D12Texture*>(texture);

		ComputeContext->CommandList->SetComputeRootDescriptorTable(1, d3d12Texture->GpuHandleSrv);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(2, OutputBufferUav);


		ComputeContext->CommandList->Dispatch(X, Y, Z);


		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(ReadBackBuffer.Get(), OutputBuffer.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CounterResource.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(CounterReadback.Get(), CounterResource.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CounterResource.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		const INT32 rawData[1] = { 0 };
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = rawData;
		subResourceData.RowPitch = sizeof(INT32);
		subResourceData.SlicePitch = subResourceData.RowPitch;

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CounterResource.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COPY_DEST
		));

		// Copy the data into the upload heap
		UpdateSubresources(ComputeContext->CommandList.Get(), CounterResource.Get(), CounterUpload.Get(), 0, 0, 1, &subResourceData);

		// Add the instruction to transition back to read 
		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CounterResource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		));

		ComputeContext->FlushComputeQueue();

		INT32* count = nullptr;

		const HRESULT	countMapResult = CounterReadback->Map(0, nullptr, reinterpret_cast<void**>(&count));
		THROW_ON_FAILURE(countMapResult);

		INT32 debug = *count;
		CounterReadback->Unmap(0, nullptr);

		Quad* data;
		const HRESULT mappingResult = ReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&data));
		THROW_ON_FAILURE(mappingResult);

		RawQuadBuffer.clear();
		RawQuadBuffer.reserve(*count);
		for (INT32 i = 0; i < *count; ++i)
		{
			RawQuadBuffer.push_back(data[i]);
		}
		ReadBackBuffer->Unmap(0, nullptr);
	}

	void DualContouring::BuildRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE table0;
		table0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE table1;
		table1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[3];
		slotRootParameter[0].InitAsConstants(5, 0);					// world settings view
		slotRootParameter[1].InitAsDescriptorTable(1, &table0);		// density texture buffer 
		slotRootParameter[2].InitAsDescriptorTable(1, &table1);		// output buffer 

		// A root signature is an array of root parameters.
		const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_NONE
		);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature
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

	void DualContouring::BuildDualContourPipelineState()
	{
		DualPipelineStateObj = PipelineStateObject::Create(ComputeContext, DualShader.get(), RootSignature);
	}

	void DualContouring::CreateBufferCounter()
	{
		CounterResource = D3D12BufferUtils::CreateCounterResource(true, true);
		CounterReadback = D3D12BufferUtils::CreateReadBackBuffer(4);
		D3D12BufferUtils::CreateUploadBuffer(CounterUpload, 4);
	}

	void DualContouring::CreateOutputBuffer()
	{
		auto const bufferWidth = NumberOfBufferElements * sizeof(Quad);
		OutputBuffer = D3D12BufferUtils::CreateStructuredBuffer(bufferWidth, true, true);
		ReadBackBuffer = D3D12BufferUtils::CreateReadBackBuffer(bufferWidth);

		/** create views for the vertex buffer */
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.StructureByteStride = sizeof(Quad);
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = NumberOfBufferElements;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		OutputBufferUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, OutputBuffer.Get(), CounterResource.Get());
	}

	
}