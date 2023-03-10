#pragma once
#include "Platform/DirectX12/DirectX12.h"

namespace Engine
{
	class D3D12Context;
	using Microsoft::WRL::ComPtr;

	class D3D12CopyContext
	{
	public:

		static void Init(D3D12Context* context);

		static void ResetCopyList(ID3D12PipelineState* state);

		static void ExecuteCopyList();

		static void CopyTexture
		(
			ID3D12Resource* src, ID3D12Resource* dst,
			INT32 x, INT32 y, INT32 z
		);

		static void CopyBuffer
		(
			ID3D12Resource* src, UINT64 srcOffset,
			ID3D12Resource* dst, UINT64 dstOffset,
			UINT64 sizeInBytes
		);



	private:
		static UINT64 FenceValue;
		static D3D12Context* Context;

		static ComPtr<ID3D12CommandAllocator>		Allocator;
		static ComPtr<ID3D12GraphicsCommandList>	CmdList;
		static ComPtr<ID3D12CommandQueue>			Queue;
	};


}
