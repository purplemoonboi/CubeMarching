#pragma once
#include <Framework/Core/core.h>
#include <Framework/Core/Compute/ComputeApi.h>

#include "Platform/DirectX12/DirectX12.h"

#include <array>


namespace Foundation::Graphics::D3D12
{
	class D3D12Context;

	constexpr UINT NUMBER_OF_CS_FRAMES_IN_FLIGHT = 1;

	struct D3D12ComputeFrameResource
	{
		D3D12ComputeFrameResource(ID3D12Device* device);
		DISABLE_COPY_AND_MOVE(D3D12ComputeFrameResource);
		~D3D12ComputeFrameResource();


		ComPtr<ID3D12CommandAllocator> CommandAllocator;
		UINT64 Fence = 0;
	};

	class D3D12ComputeApi : public ComputeApi
	{
	public:
		~D3D12ComputeApi() override {}

		D3D12Context* Context;

		void Init(GraphicsContext* context) override;

		void ResetComputeCommandList(PipelineStateObject* state) override;

		void ExecuteComputeCommandList(UINT64* fence) override;
		void FlushComputeQueue(UINT64* fence) override;
		void Wait(UINT64* fence) override;
		void GlobalSignal(UINT64* fence) override;

		std::array<ScopePointer<D3D12ComputeFrameResource>, NUMBER_OF_CS_FRAMES_IN_FLIGHT> CsFrameResources;
		D3D12ComputeFrameResource* CurrentCSFrameResource = nullptr;
		UINT32 CurrentFrameResourceIndex = 0;

		ComPtr<ID3D12GraphicsCommandList> CommandList;
		ComPtr<ID3D12CommandAllocator> CommandAllocator;
		ComPtr<ID3D12CommandQueue> Queue;

		ComPtr<ID3D12Fence> Fence;

		PipelineStateObject* LastPso = nullptr;

		UINT64 FenceValue = 0;

	};
}
