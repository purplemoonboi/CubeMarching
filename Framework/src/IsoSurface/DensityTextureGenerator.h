#pragma once
#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "VoxelWorldConstantExpressions.h"

#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"

namespace Foundation
{
	struct ShaderArgs;
	class MemoryManager;
	class D3D12ReadBackBuffer;
	class D3D12HeapManager;
	class D3D12PipelineStateObject;
	class D3D12Shader;
	class Shader;


	struct PerlinNoiseSettings
	{
		float Octaves = 3.0f;
		float Gain = 3;
		float Loss = 0.48f;
		float GroundHeight = (float)ChunkHeight / 2.0f;
		XMFLOAT3 ChunkCoord = {0.0f, 0.0f, 0.0f};
		float Frequency = 0.01f;
		float Amplitude = 20.0f;//for heightmaps
		float BoundingMaxX = 128;
		float BoundingMaxY = 128;
		float BoundingMaxZ = 128;
		INT32 TextureWidth = VoxelTextureWidth;
		INT32 TextureHeight = VoxelTextureHeight;
	};

	enum class DensityPrimitives : INT32
	{
		Box = 0,
		Sphere = 1,
		Cylinder = 2,
		Torus = 3
	};

	enum class CsgOperation : INT32
	{
		Add = 0,
		Subtract = 1,
		Union = 2
	};

	struct CSGOperationSettings
	{
		XMFLOAT3 MousePos = {-1, -1, -1};
		INT32 IsMouseDown = 0;
		float Radius = 2.0f;
		INT32 DensityPrimitive = 0;
		INT32 CsgOperation = 0;
		float RayDistance = 15.0f;

	};

	class DensityTextureGenerator
	{
	public:

		void Init(ComputeApi* context, MemoryManager* memManager);

		void PerlinFBM(const PerlinNoiseSettings& noiseSettings, const CSGOperationSettings& csgSettings, Texture* volume);

		void Smooth(const CSGOperationSettings& settings);

		[[nodiscard]] Texture* GetTexture() const { return ResultTexture; }


		void Create(const PerlinNoiseSettings& settings);
	private:
		D3D12ComputeApi* ComputeContext = nullptr;
		D3D12HeapManager* MemManager = nullptr;

		ComPtr<ID3D12RootSignature> DensityRootSignature;

		ScopePointer<PipelineStateObject> PerlinFBMPso;
		ScopePointer<PipelineStateObject> SmoothPso;

		UINT64 FenceValue = 0;

		ScopePointer<Shader> PerlinShader;
		ScopePointer<Shader> SmoothShader;

		Texture* ResultTexture = nullptr;
		ScopePointer<Texture> ScalarTexture;



		void BuildComputeRootSignature();

		void BuildPipelineState();

		void BuildResource();

	};

}
