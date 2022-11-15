#pragma once
#include "Framework/Core/Core.h"

#include "RendererAPI.h"


namespace Engine
{

	class Renderer
	{
	public:
		static void Init();
		static void InitD3D(HWND windowHandle, INT32 bufferWidth, INT32 bufferHeight);
		static void OnWindowResize(INT32 x, INT32 y, INT32 width, INT32 height);
		static RendererAPI::Api GetAPI() { return RendererAPI::GetAPI(); }

		//static void Pass(const RefPointer<Shader>& shader, const RefPointer<VertexArray>& vertex_array, const glm::mat4& transform = glm::mat4(1));
	private:
		struct SceneData
		{
			
		};

		static SceneData* SceneData;

	};
}


