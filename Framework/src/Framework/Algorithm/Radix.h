#pragma once
#include <vector>
#include <Platform/DirectX12/DirectX12.h>
#include <Platform/DirectX12/Compute/D3D12ComputeApi.h>
#include <Platform/DirectX12/Allocator/D3D12HeapManager.h>

#include "IsoSurface/VoxelWorldConstantExpressions.h"


namespace Foundation
{
	using Microsoft::WRL::ComPtr;

	class PipelineStateObject;
	class Shader;
	class Texture;

	class Radix
	{
	public:
		void Init(ComputeApi* context, MemoryManager* memManager);


		void SortChunk(const VoxelWorldSettings& settings);


	private:
		D3D12ComputeApi* ComputeContext = nullptr;
		D3D12HeapManager* MemManager = nullptr;
		UINT64 FenceValue = 0;

		ComPtr<ID3D12RootSignature> RootSignature;

		ScopePointer<PipelineStateObject> ComputeMortonPso;
		ScopePointer<Shader> ComputeMortonCS;

		ScopePointer<PipelineStateObject> RadixSortPso;
		ScopePointer<Shader> RadixSortShader;

		ScopePointer<PipelineStateObject> GlobalBucketSumPso;
		ScopePointer<Shader> GlobalBucketSumCS;

		ScopePointer<PipelineStateObject> GlobalComputeDestPso;
		ScopePointer<Shader> GlobalComputeDestCS;

		void BuildRootSignature();
		void BuildResources();
		void BuildViews();


		ComPtr<ID3D12Resource> InputMortonCodes;
		ComPtr<ID3D12Resource> SortedMortonCodes;
		ComPtr<ID3D12Resource> CycleCounter;
		ComPtr<ID3D12Resource> CycleCounterReadBack;
		ComPtr<ID3D12Resource> CycleCounterUpload;

		ComPtr<ID3D12Resource> MortonUploadBuffer;
		ComPtr<ID3D12Resource> MortonReadBackBuffer;
		ComPtr<ID3D12Resource> MortonReadBackBufferB;

		ComPtr<ID3D12Resource> GlobalBuckets;
		ComPtr<ID3D12Resource> GlobalBucketsReadBack;
		ComPtr<ID3D12Resource> GlobalBucketsUpload;


		D3D12_GPU_DESCRIPTOR_HANDLE MortonCodeUav;
		D3D12_GPU_DESCRIPTOR_HANDLE SortedMortonUav;
		D3D12_GPU_DESCRIPTOR_HANDLE GlobalBucketsUav;
		D3D12_GPU_DESCRIPTOR_HANDLE CycleCounterUav;
	};
}
