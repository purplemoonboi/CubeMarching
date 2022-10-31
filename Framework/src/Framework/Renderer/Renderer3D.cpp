#include "Renderer3D.h"
#include "RenderInstruction.h"
#include "Platform/DirectX12/DX12RenderingApi.h"


namespace Engine
{
	// @brief - Contains performance stats of renderer.
	static Renderer3D::RenderingStats RenderStats;

	void Renderer3D::Init()
	{
		
	}

	void Renderer3D::Shutdown()
	{
	}


	void Renderer3D::BeginScene(MainCamera& cam)
	{
		/** update the constant buffer with new worldprojview matrix */
		const auto api = dynamic_cast<DX12RenderingApi*>(RenderInstruction::GetApiPtr());

		CORE_ASSERT(api, "Invalid conversion...");

		api->UpdateConstantBuffer(cam.GetWorldViewProjMat());

	}

	void Renderer3D::EndScene()
	{
	}

	void Renderer3D::FlushCommandQueue()
	{

	}

	void Renderer3D::DrawDemoBox()
	{
		/** for now we're going to force draw a cube. Not efficient will implement improved code soon! */
		RenderInstruction::DrawDemoScene();
	}

	Renderer3D::RenderingStats& Renderer3D::GetRenderingStats()
	{
		return RenderStats;
	}

}
