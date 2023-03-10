#pragma once
#include "Framework/Core/Compute/ComputeApi.h"
#include <Platform/DirectX12/DirectX12.h>

namespace Engine
{
	struct D3D12FrameResource;
	using Microsoft::WRL::ComPtr;
	class D3D12Context;

	class D3D12ComputeApi : public ComputeApi
	{
	public:
		~D3D12ComputeApi() override {}

		void Init(GraphicsContext* context) override;

		void ResetComputeCommandList(PipelineStateObject* state) override;

		void ExecuteComputeCommandList(UINT64* voxelWorldSyncValue) override;
		void FlushComputeQueue(UINT64* voxelWorldSyncValue) override;
		void Wait(UINT64* voxelWorldSyncValue) override;
		ComPtr<ID3D12GraphicsCommandList> CommandList;
		D3D12Context* Context = nullptr;

		ComPtr<ID3D12CommandAllocator> CommandAllocator;
		ComPtr<ID3D12CommandQueue> Queue;


		ComPtr<ID3D12Fence> Fence;

		UINT64 FenceValue = 0;

	};
}