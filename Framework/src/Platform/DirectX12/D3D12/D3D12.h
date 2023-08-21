#pragma once
#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Foundation::Graphics::D3D12
{
	class D3D12Context;

	inline ScopePointer<D3D12Context> gD3D12Context{ nullptr };

}