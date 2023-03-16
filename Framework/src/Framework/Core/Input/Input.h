#pragma once
#include "Framework/Core/core.h"

namespace Engine
{
	struct Vector2
	{
		float X = 0.f;
		float Y = 0.f;
	};


	class Input
	{
	public:
		static bool IsKeyPressed(INT32 keycode);
		static bool IsMouseButtonPressed(UINT64 button);
		static Vector2 GetMousePosition();
	};
}