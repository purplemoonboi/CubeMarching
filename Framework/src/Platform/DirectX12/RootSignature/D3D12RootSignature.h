#pragma once
#include "Platform/DirectX12/DirectX12.h"

namespace Engine
{
	using Microsoft::WRL::ComPtr;

	class D3D12Context;

	class D3D12RootSignature
	{
	public:

		void Initialise(D3D12Context* context);

		[[nodiscard]] ID3D12RootSignature* GetRootSignature() const { return RootSignature.Get(); }


	private:
		ComPtr<ID3D12RootSignature>	RootSignature;


	};
}