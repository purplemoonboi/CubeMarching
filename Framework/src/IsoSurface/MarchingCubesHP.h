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
		INT32 WorldIndex;
	};

	struct MCIEdgeElementTable
	{
		INT32 Key;
		MCIVertex Value;
	};

	struct MCIFace
	{
		INT32 I0;
		INT32 I1;
		INT32 I2;
	};

	class MarchingCubesHP
	{
	public:

		void Init(ComputeApi* context, MemoryManager* memManager);

		void RadixSort(Texture* texture);

		void SortChunk();

		void StreamMCVoxels();

		[[nodiscard]] const std::vector<Triangle>& GetTriangleBuffer() const
		{
			return RawTriangleBuffer;
		}

	private:

		D3D12ComputeApi* ComputeContext = nullptr;
		D3D12MemoryManager* MemManager = nullptr;
		UINT64 FenceValue = 0;

		void BuildRootSignature();

		ComPtr<ID3D12RootSignature> RootSignature;

		ScopePointer<PipelineStateObject> BuildUpPso;
		ScopePointer<Shader> BuildUpShader;

		ScopePointer<PipelineStateObject> PrefixOffsetPso;
		ScopePointer<Shader> PrefixOffsetShader;

		ScopePointer<PipelineStateObject> StreamPso;
		ScopePointer<Shader> StreamShader;

		ScopePointer<PipelineStateObject> MortonCodePso;
		ScopePointer<Shader> ComputeMortonCodes;

		ScopePointer<PipelineStateObject> RadixSortPso;
		ScopePointer<Shader> RadixSortShader;

		ScopePointer<PipelineStateObject> GlobalBucketSumPso;
		ScopePointer<Shader> GlobalBucketSumCS;

		ScopePointer<PipelineStateObject> GlobalComputeDestPso;
		ScopePointer<Shader> GlobalComputeDestCS;

		ScopePointer<PipelineStateObject> GenerateVerticesPso;
		ScopePointer<Shader> GenerateVerticesCS;

		ScopePointer<PipelineStateObject> GenerateTrianglesPso;
		ScopePointer<Shader> GenerateTrianglesCS;

		void BuildResources();

		ComPtr<ID3D12Resource> HPResource;
		ComPtr<ID3D12Resource> HPResourceReadBack;

		ComPtr<ID3D12Resource> InputMortonCodes;
		ComPtr<ID3D12Resource> SortedMortonCodes;

		ComPtr<ID3D12Resource> MortonUploadBuffer;
		ComPtr<ID3D12Resource> MortonReadBackBuffer;
		ComPtr<ID3D12Resource> MortonReadBackBufferB;

		ComPtr<ID3D12Resource> HistogramResoure;
		ComPtr<ID3D12Resource> HistogramReadBack;


		ComPtr<ID3D12Resource> FaceBuffer;
		ComPtr<ID3D12Resource> FaceReadBackBuffer;

		ComPtr<ID3D12Resource> VertexBuffer;
		ComPtr<ID3D12Resource> VertexReadBackBuffer;

		ComPtr<ID3D12Resource> IndicesBuffer;
		ComPtr<ID3D12Resource> IndicesReadBackBuffer;

		ComPtr<ID3D12Resource> HashMapBuffer;
		ComPtr<ID3D12Resource> HashMapReadBackBuffer;

		ComPtr<ID3D12Resource> LookUpTableResource;
		ComPtr<ID3D12Resource> LookUpTableUpload;

		ComPtr<ID3D12Resource> VertexCounter;
		ComPtr<ID3D12Resource> VertexCounterReadBack;
		ComPtr<ID3D12Resource> VertexCounterUpload;


		void BuildViews();

		D3D12_GPU_DESCRIPTOR_HANDLE LookUpTableSrv;

		D3D12_GPU_DESCRIPTOR_HANDLE HPResourceUav;
		D3D12_GPU_DESCRIPTOR_HANDLE TriResourceUav;
		D3D12_GPU_DESCRIPTOR_HANDLE MortonCodeUav;
		D3D12_GPU_DESCRIPTOR_HANDLE SortedMortonUav;
		D3D12_GPU_DESCRIPTOR_HANDLE HistogramUav;
		D3D12_GPU_DESCRIPTOR_HANDLE CycleCounterUav;

		D3D12_GPU_DESCRIPTOR_HANDLE FaceUav;
		D3D12_GPU_DESCRIPTOR_HANDLE VertexUav;
		D3D12_GPU_DESCRIPTOR_HANDLE IndexUav;
		D3D12_GPU_DESCRIPTOR_HANDLE HashMapUav;
		D3D12_GPU_DESCRIPTOR_HANDLE VertexCounterUav;


		std::vector<Triangle> RawTriangleBuffer;

	};
}