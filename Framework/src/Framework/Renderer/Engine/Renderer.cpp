#include "Framework/cmpch.h"
#include "Renderer.h"
#include "Renderer3D.h"
#include "RenderInstruction.h"


namespace Engine
{
	RendererStatus Renderer::RenderStatus = RendererStatus::RUNNING;

	void Renderer::Init()
	{
		RenderInstruction::Init();

		Renderer3D::Init();

	}

	void Renderer::InitD3D(HWND windowHandle, INT32 bufferWidth, INT32 bufferHeight)
	{
		// Initialise the api
		RenderInstruction::InitD3D(windowHandle, bufferWidth, bufferHeight);

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

	
}
