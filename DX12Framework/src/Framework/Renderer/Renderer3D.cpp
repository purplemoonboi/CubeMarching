#include "Renderer3D.h"

namespace DX12Framework
{
	// @brief - Contains performance stats of renderer.
	static Renderer3D::RenderingStats RenderStats;

	void Renderer3D::Init()
	{
	}

	void Renderer3D::Shutdown()
	{
	}

	void Renderer3D::BeginScene(Camera& camera)
	{
	}

	void Renderer3D::EndScene()
	{
	}

	void Renderer3D::FlushCommandQueue()
	{

	}

	void Renderer3D::Draw()
	{
		
	}

	Renderer3D::RenderingStats& Renderer3D::GetRenderingStats()
	{
		return RenderStats;
	}

}
