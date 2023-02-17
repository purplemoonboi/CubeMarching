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
		float Octaves = 8;
		float Gain = 2.0f;
		float Loss = 0.5f;
		float Ground = ChunkWidth / 2;
	};

	class PerlinCompute
	{
	public:

		void Init(ComputeApi* context, MemoryManager* memManager, ShaderArgs args);


		void Dispatch(PerlinNoiseSettings args, UINT X, UINT Y, UINT Z);

		[[nodiscard]] Texture* GetTexture() const { return ScalarTexture.get(); }

		[[nodiscard]] const std::vector<float>& GetRawTexture() const { return RawTexture; }

	private:
		D3D12ComputeApi* ComputeContext;
		D3D12MemoryManager* MemManager;

		ComPtr<ID3D12RootSignature> ComputeRootSignature;
		ComPtr<ID3D12PipelineState> Pso;

		ScopePointer<Shader> PerlinShader;
		ScopePointer<Texture> ScalarTexture;
		ComPtr<ID3D12Resource> ReadBackBuffer;

		std::vector<float> RawTexture;

		void BuildComputeRootSignature();

		void BuildPipelineState();

		void BuildResource();

		void CreateReadBackBuffer();
	};

}
