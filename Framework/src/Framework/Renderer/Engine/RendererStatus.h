#pragma once
#include <intsafe.h>

namespace Engine
{
	//TODO: Make these more descriptive, add functionality for handling states.
	enum class RendererStatus : UINT8
	{
		CRITICAL_FAILURE = 0,
		INITIALISING,
		FINISHED_INITIALISING,
		PAUSED,
		RUNNING,
		INVALIDATING_BUFFER
	};
}
