#pragma once
#include <intsafe.h>


namespace Foundation::Graphics::D3D12
{
	inline constexpr INT32 SWAP_CHAIN_BUFFER_COUNT = 2;
	inline constexpr INT32 FRAMES_IN_FLIGHT = 1;
	inline constexpr INT32 MAX_D3D12_WORLD_OBJECT_COUNT = 4096 * FRAMES_IN_FLIGHT;

	inline constexpr UINT32 GBUFFER_SIZE = 5;

}
