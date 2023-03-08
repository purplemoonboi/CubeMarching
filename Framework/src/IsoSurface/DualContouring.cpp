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
			L"assets\\shaders\\DualContouringOctree.hlsl",
			"ComputeMaterials",
			"cs_5_0"
		};
		ComputeMaterials = Shader::Create(materialPass.FilePath, materialPass.EntryPoint, materialPass.ShaderModel);

		const ShaderArgs cornersPass =
		{
			L"assets\\shaders\\DualContouringOctree.hlsl",
			"ComputeCorners",
			"cs_5_0"
		};
		ComputeCorners = Shader::Create(cornersPass.FilePath, cornersPass.EntryPoint, cornersPass.ShaderModel);

		const ShaderArgs addLengthPass =
		{
			L"assets\\shaders\\DualContouringOctree.hlsl",
			"AddLength",
			"cs_5_0"
		};
		ComputeAddLength = Shader::Create(addLengthPass.FilePath, addLengthPass.EntryPoint, addLengthPass.ShaderModel);

		const ShaderArgs positionsPass =
		{
			L"assets\\shaders\\DualContouringOctree.hlsl",
			"ComputePositions",
			"cs_5_0"
		};
		ComputePositions = Shader::Create(positionsPass.FilePath, positionsPass.EntryPoint, positionsPass.ShaderModel);

		const ShaderArgs voxelPass =
		{
			L"assets\\shaders\\DualContouringOctree.hlsl",
			"ComputeVoxels",
			"cs_5_0"
		};
		ComputeVoxels = Shader::Create(voxelPass.FilePath, voxelPass.EntryPoint, voxelPass.ShaderModel);

		BuildRootSignature();
		BuildDualContourPipelineStates();
		CreateBuffers();

	}

	void DualContouring::Dispatch(VoxelWorldSettings& settings, Texture* texture, INT32 X, INT32 Y, INT32 Z)
	{
		ComputeContext->ResetComputeCommandList(ComputeCornersPso.get());

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetShaderResourceDescHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		const UINT groupXZ = ChunkWidth   / settings.OctreeSize;
		const UINT groupY  = ChunkHeight / settings.OctreeSize; 

		/* first pass - compute the density values at the corners of each voxel */


		auto ps = dynamic_cast<D3D12PipelineStateObject*>(ComputeCornersPso.get());
		ComputeContext->CommandList->SetPipelineState(ps->GetPipelineState());

		const float ccoord[] = { settings.ChunkCoord.x, settings.ChunkCoord.y, settings.ChunkCoord.z };
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 3, &ccoord, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.NumOfPointsPerAxis, 12);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.NumOfPointsPerAxis, 4);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.PrimitiveCount, 4);

		ComputeContext->CommandList->SetGraphicsRootDescriptorTable(1, CornerMaterialsUav);
		ComputeContext->CommandList->SetGraphicsRootDescriptorTable(2, VoxelMaterialsUav);
		ComputeContext->CommandList->SetGraphicsRootDescriptorTable(5, CornerCountUav);

		ComputeContext->CommandList->Dispatch(groupXZ, groupY, groupXZ);

		/* second pass - append the number of corners that were processed per node */

		ps = dynamic_cast<D3D12PipelineStateObject*>(ComputeAddLengthPso.get());
		ComputeContext->CommandList->SetPipelineState(ps->GetPipelineState());

		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 3, &ccoord, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.NumOfPointsPerAxis, 12);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.NumOfPointsPerAxis, 4);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.PrimitiveCount, 4);

		ComputeContext->CommandList->SetComputeRootDescriptorTable(5, CornerCountUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(6, FinalCountUav);

		//TODO: This pass really doesn't benefit from the compute shader... does it?
		//TODO: ...could append the process onto the end of the first pass?
		//TODO: ...or even just map the buffers on the CPU and loop through the
		//TODO: processed nodes?
	
		ComputeContext->CommandList->Dispatch(1, 1, 1);

		/* third pass - compute the node positions */
		ps = dynamic_cast<D3D12PipelineStateObject*>(ComputePositionsPso.get());
		ComputeContext->CommandList->SetPipelineState(ps->GetPipelineState());

		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 3, &ccoord, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.NumOfPointsPerAxis, 12);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.NumOfPointsPerAxis, 4);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.PrimitiveCount, 4);

		ComputeContext->CommandList->SetComputeRootDescriptorTable(2, VoxelMaterialsUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(3, CornerIndexesUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(5, CornerCountUav);

		//TODO: Update this
		ComputeContext->CommandList->Dispatch(1, 1, 1);

		/* fourth pass - compute the vertex locations using a QEF */
		ps = dynamic_cast<D3D12PipelineStateObject*>(ComputeVoxelsPso.get());
		ComputeContext->CommandList->SetPipelineState(ps->GetPipelineState());

		ComputeContext->CommandList->SetComputeRootDescriptorTable(0, VoxelBufferUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(1, CornerMaterialsUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(2, VoxelMaterialsUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(3, CornerIndexesUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(4, VoxelMinsUav);

		ComputeContext->CommandList->Dispatch(128/settings.OctreeSize,1,1);

		/* flush the instructions */
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
		ComputeAddLengthPso = PipelineStateObject::Create(ComputeContext, ComputeAddLength.get(), RootSignature);
		ComputePositionsPso = PipelineStateObject::Create(ComputeContext, ComputePositions.get(), RootSignature);
		ComputeVoxelsPso	= PipelineStateObject::Create(ComputeContext, ComputeVoxels.get(),	  RootSignature);
	}

	void DualContouring::CreateBuffers()
	{
		/* create views for the vertex buffer */
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.StructureByteStride = sizeof(Quad);
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = VoxelWorldElementCount;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		/* create the voxel buffer */
		auto const bufferWidth = DualBufferCapacity * sizeof(GPUVoxel);
		VoxelBuffer						= D3D12BufferUtils::CreateStructuredBuffer(bufferWidth, true, true);
		VoxelReadBackBuffer				= D3D12BufferUtils::CreateReadBackBuffer(bufferWidth);
		VoxelCounterBuffer				= D3D12BufferUtils::CreateCounterResource(true, true);
		VoxelBufferUav					= D3D12Utils::CreateUnorderedAccessView(uavDesc, VoxelBuffer.Get(), VoxelCounterBuffer.Get());

		/* create the buffer to hold the densities */
		uavDesc.Buffer.StructureByteStride = sizeof(UINT32);
		const UINT64 materialCapacity = VoxelWorldElementCount * sizeof(UINT32);
		CornerMaterials					= D3D12BufferUtils::CreateStructuredBuffer(materialCapacity, true, true);
		CornerMaterialsReadBackBuffer	= D3D12BufferUtils::CreateReadBackBuffer(materialCapacity);
		CornerMaterialsUav				= D3D12Utils::CreateUnorderedAccessView(uavDesc, CornerMaterials.Get());

		VoxelMaterialsBuffer			= D3D12BufferUtils::CreateStructuredBuffer(materialCapacity, true, true);
		VoxelMaterialsReadBackBuffer	= D3D12BufferUtils::CreateReadBackBuffer(materialCapacity);
		VoxelMaterialsUav				= D3D12Utils::CreateUnorderedAccessView(uavDesc, VoxelMaterialsBuffer.Get());

		CornerIndexesBuffer				= D3D12BufferUtils::CreateStructuredBuffer(512, true, true);
		CornerIndexesReadBackBuffer		= D3D12BufferUtils::CreateReadBackBuffer(512);
		CornerIndexesUav				= D3D12Utils::CreateUnorderedAccessView(uavDesc, CornerIndexesBuffer.Get());

		/* update the stride size in bytes	&  recalculate the size of the buffer in bytes */
		uavDesc.Buffer.StructureByteStride = sizeof(float) * 3;
		const UINT64 voxelMinsCapcity = VoxelWorldElementCount * sizeof(float) * 3;

		VoxelMinsBuffer					= D3D12BufferUtils::CreateStructuredBuffer(voxelMinsCapcity, true, true);
		VoxelMinsReadBackBuffer			= D3D12BufferUtils::CreateReadBackBuffer(512);
		VoxelMinsUav					= D3D12Utils::CreateUnorderedAccessView(uavDesc, VoxelMinsBuffer.Get());

		const UINT64 densityBufferCapacity = sizeof(DensityPrimitive) * DensityPrimitiveCount;
		uavDesc.Buffer.StructureByteStride = sizeof(DensityPrimitive);

		DensityPrimitivesBuffer			= D3D12BufferUtils::CreateStructuredBuffer(densityBufferCapacity, true, true);
		DensityPrimitivesBackBuffer		= D3D12BufferUtils::CreateReadBackBuffer(densityBufferCapacity);
		DensityPrimitivesUav			= D3D12Utils::CreateUnorderedAccessView(uavDesc, DensityPrimitivesBuffer.Get());

		/* build the corner count buffer and the final counter buffer  */
		CornerCountBuffer				= D3D12BufferUtils::CreateStructuredBuffer(512, true, true);
		CornerCountCounterBuffer		= D3D12BufferUtils::CreateCounterResource(true, true);
		CornerCountBackBuffer			= D3D12BufferUtils::CreateReadBackBuffer(512);
		CornerCountUav					= D3D12Utils::CreateUnorderedAccessView(uavDesc, CornerCountBuffer.Get(),CornerCountCounterBuffer.Get());

		FinalCount						= D3D12BufferUtils::CreateStructuredBuffer(4, true, true);
		FinalCountCounterBuffer			= D3D12BufferUtils::CreateCounterResource(true, true);
		FinalCountReadBackBuffer		= D3D12BufferUtils::CreateReadBackBuffer(4);
		FinalCountUav					= D3D12Utils::CreateUnorderedAccessView(uavDesc, FinalCount.Get(),FinalCountCounterBuffer.Get());

	}


	
}