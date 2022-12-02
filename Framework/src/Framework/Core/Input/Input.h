#pragma once
#include "Framework/Core/core.h"

namespace Engine
{
	class Input
	{
	public:
		static bool IsKeyPressed(INT32 keycode);
		static bool IsMouseButtonPressed(UINT64 button);
		static std::pair<float, float> GetMousePosition();
	};
}