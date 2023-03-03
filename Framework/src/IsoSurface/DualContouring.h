#pragma once
#include "Framework/Core/Core.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"

#include "IsoSurface/VoxelWorldConstantExpressions.h"

namespace Engine
{

	class Texture;

	class DualContouring
	{
	public:

		void Init(ComputeApi* compute, MemoryManager* memManger, ShaderArgs& args);

		void Dispatch(VoxelWorldSettings& settings, Texture* texture, INT32 X, INT32 Y, INT32 Z);

	private:
		D3D12ComputeApi* ComputeContext;
		D3D12MemoryManager* MemManager = nullptr;

		std::vector<Quad> RawQuadBuffer;

		void BuildRootSignature();
		ComPtr<ID3D12RootSignature> RootSignature;

		void BuildDualContourPipelineState();
		ScopePointer<PipelineStateObject> DualPipelineStateObj;
		ScopePointer<Shader> DualShader;

		void CreateOutputBuffer();
		ComPtr<ID3D12Resource> OutputBuffer;
		ComPtr<ID3D12Resource> ReadBackBuffer;
		D3D12_GPU_DESCRIPTOR_HANDLE OutputBufferUav;

		void CreateBufferCounter();
		ComPtr<ID3D12Resource> CounterResource;
		ComPtr<ID3D12Resource> CounterReadback;
		ComPtr<ID3D12Resource> CounterUpload;

	};
}