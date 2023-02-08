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
		CBSettings WorldSettings;

		VoxelWorld() = default;

		bool Init(GraphicsContext* context, MemoryManager* memManager);

		MCTriangle* GenerateChunk(DirectX::XMFLOAT3 chunkID, Texture* texture);

		D3D12Context* Context = nullptr;
		D3D12MemoryManager* MemManager = nullptr;


		ComPtr<ID3D12RootSignature> ComputeRootSignature;
		ComPtr<ID3D12PipelineState> ComputeState;

		ScopePointer<Shader> ComputeShader;
		ComPtr<ID3D12Resource> ReadBackBuffer;

		ComPtr<ID3D12Resource> OutputBuffer;
		ComPtr<ID3D12Resource> CounterResource;


		CD3DX12_CPU_DESCRIPTOR_HANDLE OutputVertexUavCpu;
		CD3DX12_GPU_DESCRIPTOR_HANDLE OutputVertexUavGpu;


		std::vector<float> RawScalarTexture;

		MCTriangle* RawTriBuffer;

		ScopePointer<D3D12UploadBuffer<MCData>> TriangulationTable;
		CD3DX12_GPU_DESCRIPTOR_HANDLE ConstantBufferCbv;
		MCData TriTableRawData;

	private:


		void BuildComputeRootSignature();

		void BuildPso();

		void CreateOutputBuffer();

		void CreateReadBackBuffer();

		void CreateConstantBuffer();

	};

}
