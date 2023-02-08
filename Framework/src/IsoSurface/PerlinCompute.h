#pragma once
#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "VoxelWorldConstantExpressions.h"

namespace Engine
{
	class MemoryManager;
	class D3D12ReadBackBuffer;
	class D3D12MemoryManager;
	class D3D12PipelineStateObject;
	class D3D12Shader;
	class Shader;

	struct PerlinArgs
	{
		float Octaves = 8;
		float Gain = 2.0f;
		float Loss = 0.5f;

	};

	class PerlinCompute
	{
	public:

		void Init(GraphicsContext* context, MemoryManager* memManager = nullptr);


		void Generate3DTexture(PerlinArgs args, UINT X, UINT Y, UINT Z);


		D3D12Context* Context;
		D3D12MemoryManager* MemManager;

		ComPtr<ID3D12RootSignature> ComputeRootSignature;
		ComPtr<ID3D12PipelineState> Pso;

		ScopePointer<Shader> PerlinShader;
		ScopePointer<Texture> ScalarTexture;
		ComPtr<ID3D12Resource> ReadBackBuffer;

		std::vector<float> RawTexture;

	private:

		void BuildComputeRootSignature();

		void BuildPipelineState();

		void BuildResource();

		void CreateReadBackBuffer();
	};

}
