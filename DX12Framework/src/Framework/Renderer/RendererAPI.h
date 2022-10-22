#pragma once

#include "DirectXMath.h"

namespace DX12Framework
{
	class RendererAPI
	{
	public:

		enum class API
		{

			None = 0,
			OpenGL = 1,
			Vulkan = 2,
			DX11 = 3,
			DX12 = 4,

		};


	public:

		virtual void Init() = 0;

		virtual void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) = 0;

		virtual void SetClearColour(DirectX::XMFLOAT4 colour) = 0;

		virtual void Clear() = 0;

		//virtual void DrawIndexed(const RefPointer<VertexArray>& vertex_array, INT32 index_count = 0) = 0;

		inline static API GetAPI() { return RendererAPIPtr; }

	private:

		static API RendererAPIPtr;

	};
}

