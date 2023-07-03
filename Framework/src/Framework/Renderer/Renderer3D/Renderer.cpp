#include "Framework/cmpch.h"
#include "Renderer.h"
#include "Renderer3D.h"
#include "RenderInstruction.h"

#include "Framework/Core/Compute/ComputeInstruction.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Foundation::Graphics
{
	RendererStatus Renderer::RenderStatus = RendererStatus::RUNNING;
	ScopePointer<GraphicsContext> Renderer::Context = nullptr;


	void Renderer::Init(Window* window)
	{
		// Initialise the api
		Context = GraphicsContext::Create(window);

		RenderInstruction::Init(Context.get());

		RenderStatus = RendererStatus::INITIALISING;


		Renderer3D::PreInit();

		Renderer3D::Init();

		Renderer3D::PostInit();
	}

	void Renderer::OnWindowResize(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		RenderStatus = RendererStatus::INVALIDATING_BUFFER;
		RenderInstruction::SetViewport(x, y, width, height);
		RenderStatus = RendererStatus::RUNNING;
	}

	RendererStatus Renderer::RendererStatus()
	{
		return RenderStatus;
	}

	void Renderer::Clean()
	{
		RenderInstruction::Clean();

		Context->Clean();
	}
}
