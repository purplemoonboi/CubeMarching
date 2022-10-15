#pragma once
#include "RendererAPI.h"

namespace DX12Framework
{

	class RenderInstruction
	{
	public:

		inline static void Init()
		{
			RendererAPIPtr->Init();
		}

		inline static void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
		{
			//RendererAPIPtr->SetViewport(x, y, Width, Height);
		}

		inline static void SetClearColour(const DirectX::XMFLOAT4& colour)
		{
			//RendererAPIPtr->SetClearColour(colour);
		}
		
		inline static void Clear()
		{
			RendererAPIPtr->Clear();
		}

		//inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertex_array, INT32 count = 0)
		//{
			//RendererAPIPtr->DrawIndexed(vertex_array, count);
		//}

	private:

		static RendererAPI* RendererAPIPtr;

	};
}


