#pragma once
#include <DirectXMath.h>
#include <intsafe.h>
#include <memory>
#include <utility>

#include <Platform/DirectX12/DirectX12.h>
#include <Platform/DirectX12/Textures/D3D12Texture.h>
#include <Platform/DirectX12/Shaders/D3D12Shader.h>
#include "Platform/DirectX12/Buffers/D3D12Buffers.h"

namespace Engine
{


	using Microsoft::WRL::ComPtr;

	class D3D12Texture;
	class D3D12Context;

	struct MCVertex
	{
		DirectX::XMFLOAT3 Position	= { 0.f, 0.f, 0.f };
		DirectX::XMFLOAT3 Normal	= { 0.f, 0.f, 0.f };
		DirectX::XMFLOAT2 TexCoords = { 0.f, 0.f };
	};

	struct MCTriangle
	{
		MCVertex VertexA;
		MCVertex VertexB;
		MCVertex VertexC;
	};

	struct CustomStruct
	{
		float scalar = 0;
		DirectX::XMFLOAT3 vec = { 0,0,0 };
	};

	constexpr UINT64 VoxelWorldSize = 32;
	constexpr UINT64 NumberOfBufferElements = VoxelWorldSize * VoxelWorldSize * VoxelWorldSize;
	constexpr UINT64 VoxelWorldVertexBufferSize = NumberOfBufferElements * 5 * sizeof(MCTriangle);
	constexpr UINT64 DebugBufferSize = VoxelWorldSize * sizeof(CustomStruct);

	struct CBSettings
	{
		float IsoValue = 0;
		float PlanetRadius = 10;
		UINT32 TextureSize = VoxelWorldSize;
		UINT32 NumOfPointsPerAxis = VoxelWorldSize;
		DirectX::XMFLOAT3 ChunkCoord = { 0.f, 0.f, 0.f };
	};

	

	class VoxelWorld
	{
	public:
		CBSettings WorldSettings;

		VoxelWorld() = default;

		bool Init(GraphicsContext* context);

		MCTriangle* GenerateChunk(DirectX::XMFLOAT3 chunkID);

		D3D12Context* Context = nullptr;

		ComPtr<ID3D12DescriptorHeap> SrvUavHeap;

		ComPtr<ID3D12RootSignature> ComputeRootSignature;
		ComPtr<ID3D12PipelineState> ComputeState;

		ScopePointer<Shader> ComputeShader;

		ComPtr<ID3D12Resource> CS_Texture;
		ComPtr<ID3D12Resource> CS_Upload_Texture;

		CD3DX12_CPU_DESCRIPTOR_HANDLE ScalarFieldSrvCpu;
		CD3DX12_CPU_DESCRIPTOR_HANDLE ScalarFieldUavCpu;
		CD3DX12_GPU_DESCRIPTOR_HANDLE ScalarFieldSrvGpu;
		CD3DX12_GPU_DESCRIPTOR_HANDLE ScalarFieldUavGpu;

		ComPtr<ID3D12Resource> OutputBuffer;
		ComPtr<ID3D12Resource> CounterResource;
		ComPtr<ID3D12Resource> ReadbackBuffer;

		CD3DX12_CPU_DESCRIPTOR_HANDLE OutputVertexUavCpu;
		CD3DX12_GPU_DESCRIPTOR_HANDLE OutputVertexUavGpu;


		std::vector<float> RawScalarTexture;

		MCTriangle* RawTriBuffer;


	private:

		void BuildResourcesAndViews();

		void BuildComputeRootSignature();

		void BuildPso();

		void CreateScalarField();

		void CreateOutputBuffer();

		void CreateReadBackBuffer();
	};

}
