#pragma once
#include "Framework/Core/Compute/ComputeApi.h"
#include <Platform/DirectX12/DirectX12.h>

namespace Engine
{
	using Microsoft::WRL::ComPtr;
	class D3D12Context;

	class D3D12ComputeApi : public ComputeApi
	{
	public:
		~D3D12ComputeApi() override {}

		void Init(GraphicsContext* context) override;

		void ResetComputeCommandList(PipelineStateObject* state) override;

		void ExecuteComputeCommandList() override;

		void FlushComputeQueue() override;

		ComPtr<ID3D12GraphicsCommandList> CommandList;
		D3D12Context* Context = nullptr;


		ComPtr<ID3D12CommandAllocator> CommandAllocator;
		ComPtr<ID3D12CommandQueue> Queue;

		UINT64 FenceValue = 0;

	};
}