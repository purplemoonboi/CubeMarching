#include "Framework/cmpch.h"
#include "Renderer.h"
#include "Renderer3D.h"
#include "RenderInstruction.h"


namespace Engine
{

	void Renderer::Init()
	{
		RenderInstruction::Init();

		Renderer3D::Init();

	}

	void Renderer::InitD3D(HWND windowHandle, INT32 bufferWidth, INT32 bufferHeight)
	{
		// Initialise the api
		RenderInstruction::InitD3D(windowHandle, bufferWidth, bufferHeight);


		Renderer3D::PreInit();

			Renderer3D::Init();

				Renderer3D::PostInit();
	}

	void Renderer::OnWindowResize(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		RenderInstruction::SetViewport(x, y, width, height);
	}

	
}
