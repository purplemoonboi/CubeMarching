#pragma once
#include "Framework/Core/Compute/ComputeApi.h"
#include <Platform/DirectX12/DirectX12.h>

#include "Framework/Core/core.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include <array>

namespace Engine
{

	constexpr UINT NUMBER_OF_CS_FRAMES_IN_FLIGHT = 3;
	struct D3D12FrameResource;
	using Microsoft::WRL::ComPtr;
	class D3D12Context;

	struct D3D12ComputeFrameResource
	{
		D3D12ComputeFrameResource(ID3D12Device* device);
		D3D12ComputeFrameResource(const D3D12ComputeFrameResource& rhs) = delete;
		D3D12ComputeFrameResource& operator=(const D3D12ComputeFrameResource& rhs) = delete;
		D3D12ComputeFrameResource&& operator=(D3D12ComputeFrameResource&& rhs) = delete;
		~D3D12ComputeFrameResource();


		ComPtr<ID3D12CommandAllocator> CommandAllocator;
		UINT64 Fence = 0;
	};

	class D3D12ComputeApi : public ComputeApi
	{
	public:
		~D3D12ComputeApi() override {}

		void Init(GraphicsContext* context) override;

		void ResetComputeCommandList(PipelineStateObject* state) override;

		void ExecuteComputeCommandList(UINT64* voxelWorldSyncValue) override;
		void FlushComputeQueue(UINT64* voxelWorldSyncValue) override;
		void Wait(UINT64* voxelWorldSyncValue) override;
		D3D12Context* Context = nullptr;

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
