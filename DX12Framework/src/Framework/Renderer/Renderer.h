#pragma once
#include "Framework/Core/Core.h"
#include "Framework/Camera/Camera.h"

#include "RenderInstruction.h"
#include "RendererAPI.h"
#include "Shader.h"


namespace DX12Framework
{
	class Renderer
	{
	public:
		static void Init();
		static void OnWindowResize(INT32 x, INT32 y, INT32 width, INT32 height);
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		static void BeginScene(Camera& camera);
		static void EndScene();
		//static void Pass(const RefPointer<Shader>& shader, const RefPointer<VertexArray>& vertex_array, const glm::mat4& transform = glm::mat4(1));
	private:
		struct SceneData
		{
			
		};

		static ScopePointer<SceneData> SceneData;
	};
}


