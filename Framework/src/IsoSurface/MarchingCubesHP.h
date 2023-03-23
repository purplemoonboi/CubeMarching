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

	class MarchingCubesHP
	{
	public:

		void Init(ComputeApi* context, MemoryManager* memManager);

		void ConstructLBVH(Texture* texture);

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

		ScopePointer<PipelineStateObject> LBVHPso;
		ScopePointer<Shader> LBVHShader;

		ScopePointer<PipelineStateObject> PrefixSumLBVHPso;
		ScopePointer<Shader> PrefixSumLBVHShader;

		void BuildResources();

		ComPtr<ID3D12Resource> HPResource;
		ComPtr<ID3D12Resource> HPResourceReadBack;

		ComPtr<ID3D12Resource> MortonResource;
		ComPtr<ID3D12Resource> MortonResourceReadBack;

		ComPtr<ID3D12Resource> OutMortonResoure;
		ComPtr<ID3D12Resource> OutMortonReadBack;

		ComPtr<ID3D12Resource> HistogramResoure;
		ComPtr<ID3D12Resource> HistogramReadBack;



		ComPtr<ID3D12Resource> TriBufferResource;
		ComPtr<ID3D12Resource> TriReadBackResource;

		ComPtr<ID3D12Resource> LookUpTableResource;
		ComPtr<ID3D12Resource> LookUpTableUpload;

		ComPtr<ID3D12Resource> ResourceCounter;
		ComPtr<ID3D12Resource> CounterReadBack;
		ComPtr<ID3D12Resource> CounterUpload;


		void BuildViews();

		D3D12_GPU_DESCRIPTOR_HANDLE LookUpTableSrv;

		D3D12_GPU_DESCRIPTOR_HANDLE HPResourceUav;
		D3D12_GPU_DESCRIPTOR_HANDLE TriResourceUav;
		D3D12_GPU_DESCRIPTOR_HANDLE MortonCodeUav;
		D3D12_GPU_DESCRIPTOR_HANDLE OutMortonUav;
		D3D12_GPU_DESCRIPTOR_HANDLE HistogramUav;

		std::vector<Triangle> RawTriangleBuffer;

	};
}