#include "Framework/cmpch.h"
#include "Renderer.h"


namespace DX12Framework
{
	/*ScopePointer<Renderer::SceneData> Renderer::SceneData = CreateScope<Renderer::SceneData>();*/

	void Renderer::Init()
	{
		//PROFILE_FUNCTION();

		RenderInstruction::Init();
	}

	void Renderer::OnWindowResize(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		RenderInstruction::SetViewport(x, y, width, height);
	}

	void Renderer::BeginScene(Camera& camera)
	{
		/*SceneData->m_ViewProjectionMatrix = camera.ViewProjectionMatrix();*/
	}

	void Renderer::EndScene()
	{

	}

	//void Renderer::Pass(const RefPointer<Shader>& shader, const RefPointer<VertexArray>& vertexArray, const glm::mat4& transform)
	//{
	//	shader->Bind();
	//	std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("view_proj", m_SceneData->m_ViewProjectionMatrix);
	//	std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("transform", transform);
	//
	//	vertexArray->Bind();
	//	RenderInstruction::DrawIndexed(vertexArray);
	//}
}