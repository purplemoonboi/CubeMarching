#pragma once
#include <vector>
#include <Platform/DirectX12/DirectX12.h>
#include <Platform/DirectX12/Compute/D3D12ComputeApi.h>
#include <Platform/DirectX12/Allocator/D3D12MemoryManager.h>

#include "VoxelWorldConstantExpressions.h"

namespace Engine
{
	using Microsoft::WRL::ComPtr;

	class PipelineStateObject;
	class Shader;
	class Texture;

	struct OctreeNode
	{
		INT32 MortonCode;
		/* The 'Data' variable holds internal data of a node.
		 *
		 * From the MSB....
		 *
		 * The first '8' parent node index.
		 *
		 * The next '8' bits represent number of child nodes.
		 *
		 * The next '8'  bits cube configuration
		 * (For MC algos this is the lookup index)
		 * (For DC algos this can be a isValid flag)
		 *
		 * The remaining '8' bits are empty...
		 *
		 */
		INT32 Data;
	};

	struct MCIVertex
	{
		XMFLOAT3 Position;
		UINT32 WorldIndex;
	};

	struct MCIEdgeElementTable
	{
		UINT32 Key;
		MCIVertex Value;
	};

	struct MCIFace
	{
		UINT32 I0;
		UINT32 I1;
		UINT32 I2;
	};

	class MarchingCubesHP
	{
	public:

		void Init(ComputeApi* context, MemoryManager* memManager);

		void Polygonise(const VoxelWorldSettings& worldSettings, Texture* texture);


		[[nodiscard]] const std::vector<UINT16>& GetIndexBuffer() const
		{
			return RawIndexBuffer;
		}

		[[nodiscard]] const std::vector<Vertex>& GetVertexBuffer() const
		{
			return RawVertexBuffer;
		}

	private:

		D3D12ComputeApi* ComputeContext = nullptr;
		D3D12MemoryManager* MemManager = nullptr;
		UINT64 FenceValue = 0;

		void BuildRootSignature();
		void BuildResources();
		void BuildViews();

		ComPtr<ID3D12RootSignature> RootSignature;

		ScopePointer<PipelineStateObject> GenerateVerticesPso;
		ScopePointer<Shader> GenerateVerticesCS;

		ScopePointer<PipelineStateObject> GenerateTrianglesPso;
		ScopePointer<Shader> GenerateTrianglesCS;

		ScopePointer<PipelineStateObject> InitHashTablePso;
		ScopePointer<Shader> InitHashTableCS;
		bool HashTableGPUInit = false;

		ComPtr<ID3D12Resource> LookUpTableResource;
		ComPtr<ID3D12Resource> LookUpTableUpload;

		ComPtr<ID3D12Resource> FaceBuffer;
		ComPtr<ID3D12Resource> FaceReadBackBuffer;

		ComPtr<ID3D12Resource> VertexBuffer;
		ComPtr<ID3D12Resource> VertexReadBackBuffer;

		ComPtr<ID3D12Resource> HashMapBuffer;
		ComPtr<ID3D12Resource> HashMapReadBackBuffer;

		ComPtr<ID3D12Resource> VertexCounter;
		ComPtr<ID3D12Resource> VertexCounterReadBack;
		ComPtr<ID3D12Resource> VertexCounterUpload;



		D3D12_GPU_DESCRIPTOR_HANDLE LookUpTableSrv;

		D3D12_GPU_DESCRIPTOR_HANDLE FaceUav;
		D3D12_GPU_DESCRIPTOR_HANDLE VertexUav;
		D3D12_GPU_DESCRIPTOR_HANDLE HashMapUav;
		D3D12_GPU_DESCRIPTOR_HANDLE VertexCounterUav;

		MCIEdgeElementTable* HashTableData = nullptr;

		std::vector<MCIEdgeElementTable> HashTableCPU_Copy;
		std::vector<Vertex> RawVertexBuffer;
		std::vector<UINT16> RawIndexBuffer;


	};
}