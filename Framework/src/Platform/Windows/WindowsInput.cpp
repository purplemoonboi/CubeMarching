#include "Framework/cmpch.h"
#include "Framework/Core/Input/Input.h"
#include "Framework/Core/Application/Application.h"
#include <Windows.h>


namespace Engine
{
	static bool IsKeyPressed(INT32 keycode)
	{
		return (GetAsyncKeyState(keycode) & 0x8000) != 0;
	}

	static bool IsMouseButtonPressed(UINT64 button)
	{
		if ((button & MK_LBUTTON) != 0)
		{
			return true;
		}
		if ((button & MK_RBUTTON) != 0)
		{
			return true;
		}
	}

	static std::pair<float, float> GetMousePosition()
	{
		LPPOINT mousePos = {};
		auto b = GetCursorPos(mousePos);
		float pos[2];
		if(b)
		{
			pos[0] = static_cast<float>(mousePos[0].x);
			pos[1] = static_cast<float>(mousePos[0].y);
		}
		else
		{
			DWORD err = GetLastError();
		}

		return { pos[0], pos[1] };
	}
}