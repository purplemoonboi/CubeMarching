#pragma once
#include <intsafe.h>


namespace Foundation
{
#ifdef FOUNDATION_MATHS
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

#elif CM_WINDOWS_PLATFORM
#include "MathHelper.h"
	using namespace DirectX;

	typedef XMFLOAT2 Float2;
	typedef XMFLOAT3 Float3;
	typedef XMVECTOR Vector;

	typedef XMFLOAT4X4 FMatrix;
	typedef XMMATRIX Matrix;

#endif




}
