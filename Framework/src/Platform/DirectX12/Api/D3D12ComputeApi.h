#pragma once
#include <Framework/Core/Compute/ComputeApi.h>

#include <Platform/DirectX12/DirectX12.h>

namespace Engine
{
	using Microsoft::WRL::ComPtr;

	class D3D12ComputeApi : public ComputeApi
	{
	public:
		~D3D12ComputeApi() override;

		void Init(GraphicsContext* context) override;


		void ResetComputeCommandList(PipelineStateObject* state) override;

		void ExecuteComputeCommandList() override;


	private:
		ID3D12Device* Device = nullptr;

		ComPtr<ID3D12CommandAllocator> CommandAllocator;
		ComPtr<ID3D12GraphicsCommandList> ComputeCommandList;
		ComPtr<ID3D12CommandQueue> ComputeQueue;

	};
}