#pragma once
#include "Framework/Core/Core.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"

#include "IsoSurface/VoxelWorldConstantExpressions.h"

namespace Engine
{

	class Texture;

	class DualContouring
	{
		struct GPUVoxel
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT3 normal;
			int numPoints;
		};
	public:

		void Init(ComputeApi* compute, MemoryManager* memManger);

		void Dispatch(VoxelWorldSettings& settings, Texture* texture, INT32 X, INT32 Y, INT32 Z);

	private:
		D3D12ComputeApi* ComputeContext;
		D3D12MemoryManager* MemManager = nullptr;

		std::vector<GPUVoxel> RawVertexBuffer;

		void BuildRootSignature();
		ComPtr<ID3D12RootSignature> RootSignature;

		void BuildDualContourPipelineStates();
		ScopePointer<PipelineStateObject> ComputeMaterialsPso;
		ScopePointer<Shader> ComputeMaterials;

		ScopePointer<PipelineStateObject> ComputeCornersPso;
		ScopePointer<Shader> ComputeCorners;

		ScopePointer<PipelineStateObject> ComputeAddLengthPso;
		ScopePointer<Shader> ComputeAddLength;

		ScopePointer<PipelineStateObject> ComputePositionsPso;
		ScopePointer<Shader> ComputePositions;

		ScopePointer<PipelineStateObject> ComputeVoxelsPso;
		ScopePointer<Shader> ComputeVoxels;

		void CreateBuffers();
		/**
		 * @brief A buffer for storing the voxels
		 */
		ComPtr<ID3D12Resource> VoxelBuffer;
		ComPtr<ID3D12Resource> VoxelReadBackBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE VoxelBufferUav;
		/**
		 * @brief A buffer for storing the corner materials
		 */
		ComPtr<ID3D12Resource> CornerMaterials;
		ComPtr<ID3D12Resource> CornerMaterialsReadBackBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE CornerMaterialsUav;
		/**
		 * @brief A buffer for storing the voxel materials
		 */
		ComPtr<ID3D12Resource> VoxelMaterialsBuffer;
		ComPtr<ID3D12Resource> VoxelMaterialsReadBackBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE VoxelMaterialsUav;
		/**
		 * @brief A buffer for storing the index buffer
		 */
		ComPtr<ID3D12Resource> CornerIndexesBuffer;
		ComPtr<ID3D12Resource> CornerIndexesReadBackBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE CornerIndexesUav;
		/**
		 * @brief A buffer for storing the voxel mins
		 */
		ComPtr<ID3D12Resource> VoxelMinsBuffer;
		ComPtr<ID3D12Resource> VoxelMinsReadBackBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE VoxelMinsUav;
		/**
		 * @brief A buffer for storing the corner count
		 */
		ComPtr<ID3D12Resource> CornerCountBuffer;
		ComPtr<ID3D12Resource> CornerCountBackBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE CornerCountUav;
		/**
		 * @brief A buffer for storing the final count
		 */
		ComPtr<ID3D12Resource> FinalCount;
		ComPtr<ID3D12Resource> FinalCountBackBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE FinalCountUav;
		/**
		 * @brief A buffer for storing the density shapes
		 */
		ComPtr<ID3D12Resource> DensityPrimitivesBuffer;
		ComPtr<ID3D12Resource> DensityPrimitivesBackBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE DensityPrimitivesUav;

		void CreateBufferCounter();
		ComPtr<ID3D12Resource> CounterResource;
		ComPtr<ID3D12Resource> CounterReadback;
		ComPtr<ID3D12Resource> CounterUpload;

		

		const UINT64 DualBufferCapacity = (ChunkWidth - 1) * (ChunkHeight - 1) * (ChunkWidth - 1);

	};
}