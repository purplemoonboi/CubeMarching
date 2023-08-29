#include "Framework/cmpch.h"
#include "RenderInstruction.h"

#include "Platform/DirectX12/Api/D3D12RenderingApi.h"

namespace Engine
{
	RendererAPI* RenderInstruction::RendererApiPtr = new D3D12RenderingApi();
}
