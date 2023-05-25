#pragma once
#include "Framework/Core/Core.h"
#include "Framework/Renderer/Engine/Mesh.h"

#include "Platform/DirectX12/DirectX12.h"
#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"
#include "Platform/DirectX12/Allocator/D3D12HeapManager.h"

#include "IsoSurface/VoxelWorldConstantExpressions.h"

namespace Foundation
{
	using Microsoft::WRL::ComPtr;

	class Texture;
	class Shader;

	class DualContouring
	{
	public:
		void Init(ComputeApi* compute);

		void Dispatch(const VoxelWorldSettings& settings, Texture* texture);

		[[nodiscard]] void* GetVertices() const { return RawTriBuffer; }
		[[nodiscard]] UINT32 GetVertexCount() const { return TriangleCount * 3; }

	private:
		D3D12ComputeApi* ComputeContext = nullptr;
		D3D12HeapManager* MemManager = nullptr;
		UINT64 FenceValue = 0;


		void BuildRootSignature();
		ComPtr<ID3D12RootSignature> RootSignature;

		void BuildDualContourPipelineStates();
		ScopePointer<PipelineStateObject> GenerateVerticesPso;
		ScopePointer<Shader> GenerateVerticesDualContourShader;

		ScopePointer<PipelineStateObject> GenerateTrianglePso;
		ScopePointer<Shader> GenerateTriangleShader;

		void CreateBuffers();
		/**
		 * @brief A buffer for storing the voxels
		 */
		ComPtr<ID3D12Resource> VertexBuffer;
		ComPtr<ID3D12Resource> VertexBackBuffer;
		ComPtr<ID3D12Resource> VertexCounterBuffer;
		ComPtr<ID3D12Resource> VertexCounterUpload;
		ComPtr<ID3D12Resource> VertexCounterReadBack;
		D3D12_GPU_DESCRIPTOR_HANDLE VertexBufferUav;

		/**
		 * @brief A buffer for holding the materials
		 */
		ComPtr<ID3D12Resource> VoxelLookUpTable;
		ComPtr<ID3D12Resource> VoxelLookUpReadBack;

		D3D12_GPU_DESCRIPTOR_HANDLE VoxelLookUpTableUav;

		/**
		 * @brief A buffer for storing the density values
		 */
		ComPtr<ID3D12Resource> TriangleBuffer;
		ComPtr<ID3D12Resource> TriangleReadBackBuffer;
		ComPtr<ID3D12Resource> TriangleCounterBuffer;
		ComPtr<ID3D12Resource> TriangleCounterUpload;
		ComPtr<ID3D12Resource> TriangleCounterReadBack;

		D3D12_GPU_DESCRIPTOR_HANDLE TriangleBufferUav;

		/**
		 * @brief Containers for raw readback
		 */

		Triangle* RawTriBuffer = nullptr;
		std::vector<Triangle> CopyTriBuffer;
		UINT32* CountData = nullptr;
		UINT32 TriangleCount = 0;

		void ResetCounters();
	};
}
