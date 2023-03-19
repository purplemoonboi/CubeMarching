#pragma once
#include <DirectXMath.h>
#include <intsafe.h>
#include <memory>
#include <utility>

#include <Platform/DirectX12/DirectX12.h>
#include <Platform/DirectX12/Textures/D3D12Texture.h>
#include <Platform/DirectX12/Shaders/D3D12Shader.h>

#include "VoxelWorldConstantExpressions.h"
#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"


namespace Engine
{
	using Microsoft::WRL::ComPtr;

	class D3D12ReadBackBuffer;
	class D3D12MemoryManager;

	class D3D12Texture;

	class MarchingCubes
	{
	public:
		MarchingCubes() = default;

		void Init(ComputeApi* context, MemoryManager* memManager);

		void Dispatch
		(
			VoxelWorldSettings const& worldSettings,
			Texture* texture
		);

		[[nodiscard]] const std::vector<Triangle>& GetTriangleBuffer() const { return RawTriBuffer; }

		[[nodiscard]] const std::vector<Vertex>& GetVertices() const { return Vertices; }

		[[nodiscard]] const std::vector<UINT16>& GetIndices() const { return Indices; }

	private:
		ID3D12Device* Device;
		D3D12ComputeApi* ComputeContext = nullptr;
		D3D12MemoryManager* MemManager = nullptr;

		ComPtr<ID3D12RootSignature> ComputeRootSignature;
		ScopePointer<PipelineStateObject> ComputeState;

		ScopePointer<Shader> ComputeShader;

		std::vector<Triangle> RawTriBuffer;


		void BuildComputeRootSignature();

		void BuildPso();

		void CreateOutputBuffer();

		void CreateCounterBuffer();

		UINT64 FenceValue = 0;

		ComPtr<ID3D12Resource> OutputBuffer;
		ComPtr<ID3D12Resource> CounterResource;
		ComPtr<ID3D12Resource> CounterReadback;
		ComPtr<ID3D12Resource> CounterUpload;

		D3D12_CPU_DESCRIPTOR_HANDLE OutputVertexUavCpu;
		D3D12_GPU_DESCRIPTOR_HANDLE OutputVertexUavGpu;

		ComPtr<ID3D12Resource> ReadBackBuffer;

		void CreateTriangulationTableBuffer();
		ComPtr<ID3D12Resource> TriangulationTable;
		ComPtr<ID3D12Resource> UploadTriangulationTable;
		D3D12_GPU_DESCRIPTOR_HANDLE TriBufferSrv;

		std::vector<Vertex> Vertices;
		std::vector<UINT16> Indices;

	};

}
