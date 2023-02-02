#pragma once
#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"


namespace Engine
{

	struct PerlinArgs
	{
		float Octaves = 8;
		float Gain = 2.0f;
		float Loss = 0.5f;

	};

	class PerlinCompute
	{
	public:

		void Init(GraphicsContext* context);


		void Generate3DTexture(PerlinArgs args);


		D3D12Context* Context;
		ComPtr<ID3D12RootSignature> ComputeRootSignature;
		std::unique_ptr<D3D12Texture> ScalarField;
		std::unique_ptr<D3D12Texture> CopyField;


	private:

		void BuildComputeRootSignature();


		void BuildComputeResourceDescriptors();

	};

}
