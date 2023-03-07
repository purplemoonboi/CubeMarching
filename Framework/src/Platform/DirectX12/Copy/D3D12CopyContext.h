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

		static void CopyTexture
		(
			ID3D12Resource* src, ID3D12Resource* dst,
			INT32 x, INT32 y, INT32 z
		);

		static void CopyBuffer
		(
			ID3D12Resource* src, ID3D12Resource* dst,
			INT32 x, INT32 y, INT32 z
		);

	private:
		static UINT64 FenceValue;
		static D3D12Context* Context;

		static ComPtr<ID3D12CommandAllocator>		Allocator;
		static ComPtr<ID3D12GraphicsCommandList>	CmdList;
		static ComPtr<ID3D12CommandQueue>			Queue;
	};


}
