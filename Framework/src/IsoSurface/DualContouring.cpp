#include "DualContouring.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"

namespace Engine
{
	void DualContouring::Init(ComputeApi* compute, MemoryManager* memManger)
	{
		ComputeContext = dynamic_cast<D3D12ComputeApi*>(compute);
		MemManager = dynamic_cast<D3D12MemoryManager*>(memManger);


		/**
		 * load and compile the shader passes
		 */
		const ShaderArgs materialPass =
		{
			L"assest\\shaders\\DualContouring.hlsl",
			"ComputeMaterials",
			"cs_5_0"
		};
		ComputeMaterials = Shader::Create(materialPass.FilePath, materialPass.EntryPoint, materialPass.ShaderModel);

		const ShaderArgs cornersPass =
		{
			L"assest\\shaders\\DualContouring.hlsl",
			"ComputeCorners",
			"cs_5_0"
		};
		ComputeCorners = Shader::Create(cornersPass.FilePath, cornersPass.EntryPoint, cornersPass.ShaderModel);

		const ShaderArgs addLengthPass =
		{
			L"assest\\shaders\\DualContouring.hlsl",
			"AddLength",
			"cs_5_0"
		};
		ComputeAddLength = Shader::Create(addLengthPass.FilePath, addLengthPass.EntryPoint, addLengthPass.ShaderModel);

		const ShaderArgs positionsPass =
		{
			L"assest\\shaders\\DualContouring.hlsl",
			"ComputePositions",
			"cs_5_0"
		};
		ComputePositions = Shader::Create(positionsPass.FilePath, positionsPass.EntryPoint, positionsPass.ShaderModel);

		const ShaderArgs voxelPass =
		{
			L"assest\\shaders\\DualContouring.hlsl",
			"ComputeVoxels",
			"cs_5_0"
		};
		ComputeVoxels = Shader::Create(voxelPass.FilePath, voxelPass.EntryPoint, voxelPass.ShaderModel);



		BuildRootSignature();
		BuildDualContourPipelineStates();
		CreateBufferCounter();
		CreateBuffers();

	}

	void DualContouring::Dispatch(VoxelWorldSettings& settings, Texture* texture, INT32 X, INT32 Y, INT32 Z)
	{
		ComputeContext->ResetComputeCommandList(ComputeMaterialsPso.get());

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetDescriptorHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		auto const d3d12Pso = dynamic_cast<D3D12PipelineStateObject*>(ComputeMaterialsPso.get());
		ComputeContext->CommandList->SetPipelineState(d3d12Pso->GetPipelineState());

		

		ComputeContext->FlushComputeQueue();

	}

	void DualContouring::BuildRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE cornerMaterials;
		cornerMaterials.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE voxelMaterials;
		voxelMaterials.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

		CD3DX12_DESCRIPTOR_RANGE cornerIndexes;
		cornerIndexes.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);

		CD3DX12_DESCRIPTOR_RANGE voxelMins;
		voxelMins.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);

		CD3DX12_DESCRIPTOR_RANGE voxels;
		voxels.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 4);

		CD3DX12_DESCRIPTOR_RANGE cornerCount;
		cornerCount.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 5);

		CD3DX12_DESCRIPTOR_RANGE finalCount;
		finalCount.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 6);


		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[8];
		slotRootParameter[0].InitAsConstants(4, 0);								// world settings view
		slotRootParameter[1].InitAsDescriptorTable(1, &cornerMaterials);		// cornerMaterials 
		slotRootParameter[2].InitAsDescriptorTable(1, &voxelMaterials);			// voxelMaterials 
		slotRootParameter[3].InitAsDescriptorTable(1, &cornerIndexes);			// corner indexes
		slotRootParameter[4].InitAsDescriptorTable(1, &voxelMins);				// voxel mins
		slotRootParameter[5].InitAsDescriptorTable(1, &voxels);					// voxels
		slotRootParameter[6].InitAsDescriptorTable(1, &cornerCount);			// cornerCount
		slotRootParameter[7].InitAsDescriptorTable(1, &finalCount);				// finalCount

		// A root signature is an array of root parameters.
		const CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(8, slotRootParameter,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_NONE
		);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		const HRESULT serializedResult = D3D12SerializeRootSignature
		(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
		}

		THROW_ON_FAILURE(serializedResult);
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

	void DualContouring::BuildDualContourPipelineStates()
	{
		ComputeMaterialsPso = PipelineStateObject::Create(ComputeContext, ComputeMaterials.get(), RootSignature);
		ComputeCornersPso	= PipelineStateObject::Create(ComputeContext, ComputeCorners.get(),   RootSignature);
		ComputeMaterialsPso = PipelineStateObject::Create(ComputeContext, ComputeMaterials.get(), RootSignature);
		ComputeAddLengthPso = PipelineStateObject::Create(ComputeContext, ComputeAddLength.get(), RootSignature);
		ComputePositionsPso = PipelineStateObject::Create(ComputeContext, ComputePositions.get(), RootSignature);
		ComputeVoxelsPso	= PipelineStateObject::Create(ComputeContext, ComputeVoxels.get(),	  RootSignature);
	}

	void DualContouring::CreateBufferCounter()
	{
		CounterResource = D3D12BufferUtils::CreateCounterResource(true, true);
		CounterReadback = D3D12BufferUtils::CreateReadBackBuffer(4);
		D3D12BufferUtils::CreateUploadBuffer(CounterUpload, 4);
	}

	void DualContouring::CreateBuffers()
	{
		auto const bufferWidth = DualBufferCapacity * sizeof(DualVertex);
		VoxelBuffer = D3D12BufferUtils::CreateStructuredBuffer(bufferWidth, true, true);
		VoxelReadBackBuffer = D3D12BufferUtils::CreateReadBackBuffer(bufferWidth);

		/** create views for the vertex buffer */
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.StructureByteStride = sizeof(Quad);
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = NumberOfBufferElements;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		VoxelBufferUav = D3D12Utils::CreateUnorderedAccessView(uavDesc, VoxelBuffer.Get(), CounterResource.Get());
	}

	
}