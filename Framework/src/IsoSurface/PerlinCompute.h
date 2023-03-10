#pragma once
#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "VoxelWorldConstantExpressions.h"

#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"

namespace Engine
{
	struct ShaderArgs;
	class MemoryManager;
	class D3D12ReadBackBuffer;
	class D3D12MemoryManager;
	class D3D12PipelineStateObject;
	class D3D12Shader;
	class Shader;

	struct PerlinNoiseSettings
	{
		float Octaves = 3.0f;
		float Gain = 3;
		float Loss = 0.48f;
		float GroundHeight = (float)ChunkHeight / 2.0f;
		DirectX::XMFLOAT3 ChunkCoord;
		float Frequency = 0.01f;
		float Amplitude = 20.0f;//for heightmaps
		float BoundingMaxX;
		float BoundingMaxY;
		float BoundingMaxZ;
	};

	class PerlinCompute
	{
	public:

		void Init(ComputeApi* context, MemoryManager* memManager, ShaderArgs args);


		void Dispatch(PerlinNoiseSettings args, UINT X, UINT Y, UINT Z);

		[[nodiscard]] Texture* GetTexture() const { return ScalarTexture.get(); }

		[[nodiscard]] const std::vector<float>& GetRawTexture() const { return RawTexture; }

	private:
		D3D12ComputeApi* ComputeContext = nullptr;
		D3D12MemoryManager* MemManager = nullptr;

		ComPtr<ID3D12RootSignature> ComputeRootSignature;
		ScopePointer<PipelineStateObject> Pso;
		//ComPtr<ID3D12PipelineState> Pso;

		UINT64 FenceValue = 0;

		ScopePointer<Shader> PerlinShader;
		ScopePointer<Texture> ScalarTexture;
		ComPtr<ID3D12Resource> ReadBackBuffer;

		std::vector<float> RawTexture;

		void BuildComputeRootSignature();

		void BuildPipelineState();

		void BuildResource();

	};

}
