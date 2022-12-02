#include "Framework/cmpch.h"
#include "Framework/Core/Input/Input.h"
#include "Framework/Core/Application/Application.h"
#include <Windows.h>


namespace Engine
{
	bool Input::IsKeyPressed(INT32 keycode)
	{
		return (GetAsyncKeyState(keycode) & 0x8000) != 0;
	}

	bool Input::IsMouseButtonPressed(UINT64 button)
	{
		
		if ((button & GetKeyState(MK_LBUTTON)) != 0)
		{
			return true;
		}
		if ((button & MK_RBUTTON) != 0)
		{
			return true;
		}

		return false;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		LPPOINT mousePos = {};
		const BOOL valid = GetCursorPos(mousePos);
		float x, y;
		if(valid)
		{
			x = static_cast<float>(mousePos[0].x);
			y = static_cast<float>(mousePos[0].y);
		}
		else
		{
			DWORD err = GetLastError();
		}

		return { x, y };
	}
}