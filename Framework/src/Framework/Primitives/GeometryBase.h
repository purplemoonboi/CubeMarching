#pragma once
#include "Framework/Core/Core.h"
#include <directxmath.h>
#include "Platform/Maths/DXMathsUtilities.h"



namespace Engine
{
	// Simple vertex buffer
	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT4 Colour;
	};

	// Simple constant buffer
	struct ObjectConstant
	{
		DirectX::XMFLOAT4X4 WorldViewProj = DXMathsUtilities::Identity4x4();
	};


	// @brief Base geometry class
	template<typename T> class Geometry
	{
		Geometry(INT32 resolution)
			:
			Resolution(resolution)
		{}

	public:

		static RefPointer<T> Create(INT32 resolution)
		{
			return CreateRef<T>(resolution);
		}


	protected:

		INT32 Resolution;
		void* Vertices;

	};
}

