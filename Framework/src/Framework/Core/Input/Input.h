#pragma once
#include "Framework/Core/core.h"
#include "KeyCodes.h"
#include "MouseButtonCodes.h"

namespace Engine
{
	class Input
	{
	public:
		static bool IsKeyPressed(INT32 keycode);
		static bool IsMouseButonPressed(INT32 button);
		static std::pair<float, float> GetMousePosition();
	};
}