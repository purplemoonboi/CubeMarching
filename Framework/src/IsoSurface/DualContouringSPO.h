#pragma once
#include "Framework/Core/Core.h"
#include "Platform/DirectX12/Allocator/D3D12HeapManager.h"
#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"

#include "IsoSurface/VoxelWorldConstantExpressions.h"

namespace Foundation
{
	struct MeshGeometry;

	class Texture;

	class DualContouringSPO
	{
		
	public:

		void Init(ComputeApi* compute);

		void Dispatch(const VoxelWorldSettings& settings, Texture* texture);

	private:

		D3D12ComputeApi* ComputeContext = nullptr;
		D3D12HeapManager* MemManager = nullptr;
		UINT64 FenceValue = 0;

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
		 * @brief A buffer for storing the density values 
		 */
		ComPtr<ID3D12Resource> CornerMaterials;
		ComPtr<ID3D12Resource> CornerMaterialsReadBackBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE CornerMaterialsUav;
		/**
		 * @brief A buffer for storing the voxel density values. 
		 */
		ComPtr<ID3D12Resource> VoxelMaterialsBuffer;
		ComPtr<ID3D12Resource> VoxelMaterialsReadBackBuffer;
		ComPtr<ID3D12Resource> VoxelCounterBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE VoxelMaterialsUav;
		/**
		 * @brief A buffer for storing the nodes configuration.
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
		 * @brief A buffer for storing the sum of the corner count from the bottom left
		 *		  node to the current thread. 
		 */
		ComPtr<ID3D12Resource> CornerCountBuffer;
		ComPtr<ID3D12Resource> CornerCountCounterBuffer;
		ComPtr<ID3D12Resource> CornerCountBackBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE CornerCountUav;
		/**
		 * @brief A buffer for storing the final count
		 */
		ComPtr<ID3D12Resource> FinalCount;
		ComPtr<ID3D12Resource> FinalCountCounterBuffer;
		ComPtr<ID3D12Resource> FinalCountReadBackBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE FinalCountUav;
		/**
		 * @brief A buffer for storing the density shapes
		 */
		ComPtr<ID3D12Resource> DensityPrimitivesBuffer;
		ComPtr<ID3D12Resource> DensityPrimitivesBackBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE DensityPrimitivesUav;

		
		std::vector<GPUVoxel*> RawVoxelBuffer;
		void CreateVertexBuffers();
		ScopePointer<MeshGeometry> TerrainMesh = nullptr;

	};
}