#include "DualContouringSPO.h"
#include "Platform/DirectX12/Pipeline/D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Utilities/D3D12Utilities.h"
#include "Framework/Renderer/Engine/Mesh.h"

namespace Engine
{
	void DualContouringSPO::Init(ComputeApi* compute, MemoryManager* memManger)
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

	void DualContouringSPO::Dispatch(const VoxelWorldSettings& settings, Texture* texture)
	{
	
		ComputeContext->Wait(&FenceValue);

		ComputeContext->ResetComputeCommandList(ComputeCornersPso.get());

		ID3D12DescriptorHeap* srvHeap[] = { MemManager->GetShaderResourceDescHeap() };
		ComputeContext->CommandList->SetDescriptorHeaps(_countof(srvHeap), srvHeap);

		const UINT groupXZ = ChunkWidth   / settings.OctreeSize;
		const UINT groupY  = ChunkHeight / settings.OctreeSize; 

		/* first pass - compute the density values at the corners of each voxel */


		auto ps = dynamic_cast<D3D12PipelineStateObject*>(ComputeCornersPso.get());
		ComputeContext->CommandList->SetPipelineState(ps->GetPipelineState());
		ComputeContext->CommandList->SetComputeRootSignature(RootSignature.Get());

		const float ccoord[] = { settings.ChunkCoord.x, settings.ChunkCoord.y, settings.ChunkCoord.z };
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 3, &ccoord, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.NumOfPointsPerAxis, 3);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.OctreeSize, 4);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.PrimitiveCount, 5);

		ComputeContext->CommandList->SetComputeRootDescriptorTable(1, CornerMaterialsUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(2, VoxelMaterialsUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(6, CornerCountUav);

		ComputeContext->CommandList->Dispatch(groupXZ, groupY, groupXZ);

		/* second pass - append the number of corners that were processed per node */

		ps = dynamic_cast<D3D12PipelineStateObject*>(ComputeAddLengthPso.get());
		ComputeContext->CommandList->SetPipelineState(ps->GetPipelineState());

		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 3, &ccoord, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.NumOfPointsPerAxis, 3);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.OctreeSize, 4);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.PrimitiveCount, 5);

		ComputeContext->CommandList->SetComputeRootDescriptorTable(5, CornerCountUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(6, FinalCountUav);

		ComputeContext->CommandList->Dispatch(1, 1, 1);

		/* third pass - compute the node positions */
		ps = dynamic_cast<D3D12PipelineStateObject*>(ComputePositionsPso.get());
		ComputeContext->CommandList->SetPipelineState(ps->GetPipelineState());

		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 3, &ccoord, 0);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.NumOfPointsPerAxis, 3);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.OctreeSize, 4);
		ComputeContext->CommandList->SetComputeRoot32BitConstants(0, 1, &settings.PrimitiveCount, 5);

		ComputeContext->CommandList->SetComputeRootDescriptorTable(2, VoxelMaterialsUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(3, CornerIndexesUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(5, CornerCountUav);

		//TODO: Update this
		ComputeContext->CommandList->Dispatch(1, 1, 1);

		/* fourth pass - compute the vertex locations using a QEF */
		ps = dynamic_cast<D3D12PipelineStateObject*>(ComputeVoxelsPso.get());
		ComputeContext->CommandList->SetPipelineState(ps->GetPipelineState());

		ComputeContext->CommandList->SetComputeRootDescriptorTable(1, CornerMaterialsUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(2, VoxelMaterialsUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(3, CornerIndexesUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(4, VoxelMinsUav);
		ComputeContext->CommandList->SetComputeRootDescriptorTable(5, VoxelBufferUav);

		ComputeContext->CommandList->Dispatch(1,1,1);

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VoxelBuffer.Get(), 
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		ComputeContext->CommandList->CopyResource(VoxelReadBackBuffer.Get(), VoxelBuffer.Get());

		ComputeContext->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VoxelBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		/* flush the instructions */
		ComputeContext->FlushComputeQueue(&FenceValue);

		GPUVoxel* voxels = nullptr;

		const HRESULT mappingResult = VoxelReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&voxels));
		THROW_ON_FAILURE(mappingResult);
		std::vector<GPUVoxel> vvoxels;
		for(INT32 i = 0; i < 128; ++i)
		{
			vvoxels.push_back(voxels[i]);
		}
		VoxelReadBackBuffer->Unmap(0, nullptr);

	}

	void DualContouringSPO::BuildRootSignature()
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
		slotRootParameter[0].InitAsConstants(6, 0);								// world settings view
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

	void DualContouringSPO::BuildDualContourPipelineStates()
	{
		ComputeMaterialsPso = PipelineStateObject::Create(ComputeContext, ComputeMaterials.get(), RootSignature);
		ComputeCornersPso	= PipelineStateObject::Create(ComputeContext, ComputeCorners.get(),   RootSignature);
		ComputeAddLengthPso = PipelineStateObject::Create(ComputeContext, ComputeAddLength.get(), RootSignature);
		ComputePositionsPso = PipelineStateObject::Create(ComputeContext, ComputePositions.get(), RootSignature);
		ComputeVoxelsPso	= PipelineStateObject::Create(ComputeContext, ComputeVoxels.get(),	  RootSignature);
	}

	void DualContouringSPO::CreateBuffers()
	{
		/* create views for the vertex buffer */
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.StructureByteStride = sizeof(GPUVoxel);
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = DualContourVoxelCapacity;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		/* create the voxel buffer */
		const UINT64 bufferWidth = DualContourVoxelCapacity * sizeof(GPUVoxel);
		VoxelBuffer						= D3D12BufferUtils::CreateStructuredBuffer(bufferWidth, true, true);
		VoxelReadBackBuffer				= D3D12BufferUtils::CreateReadBackBuffer(bufferWidth);
		VoxelCounterBuffer				= D3D12BufferUtils::CreateCounterResource(true, true);
		VoxelBufferUav					= D3D12Utils::CreateUnorderedAccessView(uavDesc, VoxelBuffer.Get(), VoxelCounterBuffer.Get());

		/* create the buffer to hold the densities */
		uavDesc.Buffer.StructureByteStride = sizeof(UINT32);
		uavDesc.Buffer.NumElements = VoxelWorldElementCount;
		const UINT64 materialCapacity = VoxelWorldElementCount * sizeof(UINT32);

		CornerMaterials					= D3D12BufferUtils::CreateStructuredBuffer(materialCapacity, true, true);
		CornerMaterialsReadBackBuffer	= D3D12BufferUtils::CreateReadBackBuffer(materialCapacity);
		CornerMaterialsUav				= D3D12Utils::CreateUnorderedAccessView(uavDesc, CornerMaterials.Get());

		/* update the stride size in bytes	&  recalculate the size of the buffer in bytes */
		constexpr UINT64 voxelMaterialsCapacity = VoxelWorldElementCount * sizeof(UINT32);
		uavDesc.Buffer.NumElements = VoxelWorldElementCount;
		uavDesc.Buffer.StructureByteStride = sizeof(UINT32);

		VoxelMaterialsBuffer			= D3D12BufferUtils::CreateStructuredBuffer(materialCapacity, true, true);
		VoxelMaterialsReadBackBuffer	= D3D12BufferUtils::CreateReadBackBuffer(materialCapacity);
		VoxelMaterialsUav				= D3D12Utils::CreateUnorderedAccessView(uavDesc, VoxelMaterialsBuffer.Get());

		constexpr UINT64 cornerIndicesCapacity = VoxelWorldElementCount * sizeof(UINT32) * 3;
		uavDesc.Buffer.NumElements = VoxelWorldElementCount;
		uavDesc.Buffer.StructureByteStride = sizeof(UINT32);

		CornerIndexesBuffer				= D3D12BufferUtils::CreateStructuredBuffer(cornerIndicesCapacity, true, true);
		CornerIndexesReadBackBuffer		= D3D12BufferUtils::CreateReadBackBuffer(cornerIndicesCapacity);
		CornerIndexesUav				= D3D12Utils::CreateUnorderedAccessView(uavDesc, CornerIndexesBuffer.Get());

		/* update the stride size in bytes	&  recalculate the size of the buffer in bytes */
		constexpr UINT64 voxelMinsCapacity = VoxelWorldElementCount * sizeof(float) * 3;
		uavDesc.Buffer.NumElements = DualContourVoxelCapacity;
		uavDesc.Buffer.StructureByteStride = sizeof(float) * 3;

		VoxelMinsBuffer					= D3D12BufferUtils::CreateStructuredBuffer(voxelMinsCapacity, true, true);
		VoxelMinsReadBackBuffer			= D3D12BufferUtils::CreateReadBackBuffer(voxelMinsCapacity);
		VoxelMinsUav					= D3D12Utils::CreateUnorderedAccessView(uavDesc, VoxelMinsBuffer.Get());

		constexpr UINT64 densityBufferCapacity = sizeof(DensityPrimitive) * DensityPrimitiveCount;
		uavDesc.Buffer.NumElements = DensityPrimitiveCount;
		uavDesc.Buffer.StructureByteStride = sizeof(DensityPrimitive);

		DensityPrimitivesBuffer			= D3D12BufferUtils::CreateStructuredBuffer(densityBufferCapacity, true, true);
		DensityPrimitivesBackBuffer		= D3D12BufferUtils::CreateReadBackBuffer(densityBufferCapacity);
		DensityPrimitivesUav			= D3D12Utils::CreateUnorderedAccessView(uavDesc, DensityPrimitivesBuffer.Get());

		const UINT64 cornerBufferCapacity = sizeof(UINT32) * DualContourVoxelCapacity;
		uavDesc.Buffer.NumElements = DualContourVoxelCapacity;
		uavDesc.Buffer.StructureByteStride = sizeof(UINT32);

		/* build the corner count buffer and the final counter buffer  */
		CornerCountBuffer				= D3D12BufferUtils::CreateStructuredBuffer(cornerBufferCapacity, true, true);
		CornerCountCounterBuffer		= D3D12BufferUtils::CreateCounterResource(true, true);
		CornerCountBackBuffer			= D3D12BufferUtils::CreateReadBackBuffer(cornerBufferCapacity);
		CornerCountUav					= D3D12Utils::CreateUnorderedAccessView(uavDesc, CornerCountBuffer.Get(),CornerCountCounterBuffer.Get());

		const UINT64 finalCountCapacity = sizeof(UINT32);
		uavDesc.Buffer.NumElements = 1;
		uavDesc.Buffer.StructureByteStride = sizeof(UINT32);

		FinalCount						= D3D12BufferUtils::CreateStructuredBuffer(finalCountCapacity, true, true);
		FinalCountCounterBuffer			= D3D12BufferUtils::CreateCounterResource(true, true);
		FinalCountReadBackBuffer		= D3D12BufferUtils::CreateReadBackBuffer(finalCountCapacity);
		FinalCountUav					= D3D12Utils::CreateUnorderedAccessView(uavDesc, FinalCount.Get(),FinalCountCounterBuffer.Get());

	}

	void DualContouringSPO::CreateVertexBuffers()
	{
		if (RawVoxelBuffer.empty())
			return;

		const HRESULT allocResult = ComputeContext->Context->Allocator->Reset();
		THROW_ON_FAILURE(allocResult);
		const HRESULT listResult = ComputeContext->Context->CmdList->Reset(ComputeContext->Context->Allocator.Get(), nullptr);
		THROW_ON_FAILURE(listResult);

		std::vector<Vertex> vertices;
		std::vector<UINT16> indices;
		UINT16 index = 0;
		for (auto & voxel : RawVoxelBuffer)
		{
			Vertex vertex;
			vertex.Position = voxel->position;
			vertex.Normal = voxel->normal;
			vertex.TexCoords = { (float)voxel->numPoints, 0 };
			vertices.push_back(vertex);
			indices.push_back(++index);
		}

		const UINT ibSizeInBytes = sizeof(UINT16) * indices.size();
		const UINT vbSizeInBytes = sizeof(Vertex) * vertices.size();

		TerrainMesh = CreateScope<MeshGeometry>("Terrain");
		TerrainMesh->VertexBuffer = VertexBuffer::Create(ComputeContext->Context, vertices.data(),
			vbSizeInBytes, vertices.size(), true);

		const BufferLayout layout =
		{
			{"POSITION",	ShaderDataType::Float3, 0, 0, false },
			{"NORMAL",		ShaderDataType::Float3, 12,1, false },
			{"TEXCOORD",	ShaderDataType::Float2, 24,2, false },
		};
		TerrainMesh->VertexBuffer->SetLayout(layout);

		TerrainMesh->IndexBuffer = IndexBuffer::Create(ComputeContext->Context, indices.data(),
			ibSizeInBytes, indices.size());

		const HRESULT closeResult = ComputeContext->Context->CmdList->Close();
		THROW_ON_FAILURE(closeResult);
		ComputeContext->Context->ExecuteGraphicsCommandList();
		ComputeContext->Context->FlushCommandQueue();

		const HRESULT deviceRemovedReason = ComputeContext->Context->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceRemovedReason);

	}
	
}