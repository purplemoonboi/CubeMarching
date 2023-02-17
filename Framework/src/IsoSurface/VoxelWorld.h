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
	class D3D12ReadBackBuffer;
	class D3D12MemoryManager;


	using Microsoft::WRL::ComPtr;

	class D3D12Texture;
	class D3D12Context;


	class VoxelWorld
	{
	public:
		VoxelWorld() = default;

		bool Init(ComputeApi* context, MemoryManager* memManager, ShaderArgs args);

		void Dispatch
		(
			VoxelWorldSettings const& worldSettings,
			DirectX::XMFLOAT3 chunkID, 
			Texture* texture
		);

		[[nodiscard]] const std::vector<Triangle>& GetTriangleBuffer() const { return RawTriBuffer; }

		[[nodiscard]] MeshGeometry* GetTerrainMesh() const { return TerrainMeshGeometry.get(); }
		
	private:

		D3D12ComputeApi* ComputeContext = nullptr;
		D3D12MemoryManager* MemManager = nullptr;


		ComPtr<ID3D12RootSignature> ComputeRootSignature;
		ComPtr<ID3D12PipelineState> ComputeState;

		ScopePointer<Shader> ComputeShader;


		std::vector<Triangle> RawTriBuffer;



		void BuildComputeRootSignature();

		void BuildPso();

		void CreateOutputBuffer();
		ComPtr<ID3D12Resource> OutputBuffer;
		ComPtr<ID3D12Resource> CounterResource;
		ComPtr<ID3D12Resource> CounterReadback;
		ComPtr<ID3D12Resource> CounterUpload;

		D3D12_CPU_DESCRIPTOR_HANDLE OutputVertexUavCpu;
		D3D12_GPU_DESCRIPTOR_HANDLE OutputVertexUavGpu;

		void CreateReadBackBuffer();
		ComPtr<ID3D12Resource> ReadBackBuffer;

		void CreateStructuredBuffer();
		ComPtr<ID3D12Resource> TriangleBuffer;
		ComPtr<ID3D12Resource> UploadTriBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE TriBufferSrv;

		void CreateVertexBuffers();
		ScopePointer<MeshGeometry> TerrainMeshGeometry;
		bool IsTerrainMeshGenerated = false;

	};

}
