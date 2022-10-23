#include "Framework/cmpch.h"
#include "Renderer.h"
#include "Renderer3D.h"


namespace DX12Framework
{

	void Renderer::Init()
	{
		RenderInstruction::Init();
		Renderer3D::Init();
	}

	void Renderer::InitD3D(HWND windowHandle, INT32 bufferWidth, INT32 bufferHeight)
	{
		RenderInstruction::InitD3D(windowHandle, bufferWidth, bufferHeight);
		Renderer3D::Init();
	}

	void Renderer::OnWindowResize(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		RenderInstruction::SetViewport(x, y, width, height);
	}

	
}