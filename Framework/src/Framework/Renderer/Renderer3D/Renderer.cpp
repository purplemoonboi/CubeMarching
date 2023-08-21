#include "Framework/cmpch.h"
#include "Renderer.h"
#include "GeometryEngine.h"
#include "RenderInstruction.h"

#include "Framework/Core/Compute/ComputeInstruction.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Foundation::Graphics
{
	RendererStatus Renderer::RenderStatus = RendererStatus::RUNNING;


	void Renderer::Init(Window* window)
	{
		

		RenderStatus = RendererStatus::INITIALISING;


		GeometryEngine::PreInit();

		GeometryEngine::Init();

		GeometryEngine::PostInit();
	}

	void Renderer::OnWindowResize(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		RenderInstruction::SetViewport(x, y, width, height);
	}

	RendererStatus Renderer::RendererStatus()
	{
		return RenderStatus;
	}

	void Renderer::Clean()
	{
		RenderInstruction::Clean();
	}
}
