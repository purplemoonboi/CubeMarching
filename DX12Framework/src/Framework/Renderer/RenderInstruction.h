#pragma once
#include "RendererAPI.h"

#include "Platform/DirectX12/DX12RenderingApi.h"

namespace DX12Framework
{

	class RenderInstruction
	{
	public:

		static void Init()
		{
			RendererApiPtr->Init();
		}

		static void InitD3D(HWND windowHandle, INT32 viewportWidth, INT32 viewportHeight)
		{
			RendererApiPtr->InitD3D(windowHandle, viewportWidth, viewportHeight);
		}

		static void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
		{
			RendererApiPtr->SetViewport(x, y, width, height);
		}

		static void SetClearColour(const DirectX::XMFLOAT4& colour)
		{
			RendererApiPtr->SetClearColour(colour);
		}

		static void Clear()
		{
			RendererApiPtr->Clear();
		}

		static void Draw()
		{
			RendererApiPtr->Draw();
		}

		//inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertex_array, INT32 count = 0)
		//{
			//RendererApiPtr->DrawIndexed(vertex_array, count);
		//}

	private:

		static RendererAPI* RendererApiPtr;

	};
}


