#pragma once
#include <intsafe.h>


namespace Foundation
{

	struct Float
	{
		float x{0.0f};
	};

	struct Vector2
	{
		float x{ 0.0f };
		float y{ 0.0f };
	};

	struct Vector3
	{
		float x{ 0.0f };
		float y{ 0.0f };
		float z{ 0.0f };
	};

	struct Vector4
	{
		float x{ 0.0f };
		float y{ 0.0f };
		float z{ 0.0f };
		float w{ 0.0f };
	};

	struct IVector2
	{
		INT32 x{ 0 };
		INT32 y{ 0 };
	};

	struct IVector3
	{
		INT32 x{ 0 };
		INT32 y{ 0 };
		INT32 z{ 0 };
	};

	struct IVector4
	{
		INT32 x{ 0 };
		INT32 y{ 0 };
		INT32 z{ 0 };
		INT32 w{ 0 };
	};
}
