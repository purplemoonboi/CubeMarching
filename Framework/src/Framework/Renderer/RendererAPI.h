#pragma once
#include <Windows.h>

namespace Engine
{
	class RendererAPI
	{
	public:

		enum class Api
		{

			None = 0,
			OpenGL = 1,
			Vulkan = 2,
			DX11 = 3,
			DX12 = 4,

		};


	public:

		virtual void Init() = 0;

		virtual void InitD3D(HWND windowHandle, INT32 viewportWidth, INT32 viewportHeight) = 0;

		virtual void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) = 0;

		virtual void SetClearColour(const float colour[4]) = 0;

		virtual void Draw() = 0;

		virtual void Clear() = 0;

		//virtual void DrawIndexed(const RefPointer<VertexArray>& vertex_array, INT32 index_count = 0) = 0;

		static Api GetAPI() { return RenderingApi; }

	private:

		static Api RenderingApi;

	};
}


