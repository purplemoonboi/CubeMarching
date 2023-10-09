#pragma once
#include <intsafe.h>

namespace Foundation::Graphics
{
	inline constexpr UINT FRAMES_IN_FLIGHT = 2;

	inline constexpr INT32 SWAP_CHAIN_BUFFER_COUNT = 2;

	inline constexpr INT32 DOUBLE_BUFFERING = 2;
	inline constexpr INT32 TRIPLE_BUFFERING = 3;

	inline constexpr UINT32 GBUFFER_SIZE = 5;
}