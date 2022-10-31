#include "Framework/cmpch.h"
#include "RenderInstruction.h"

#include "Platform/DirectX12/DX12RenderingApi.h"

namespace Engine
{
	RendererAPI* RenderInstruction::RendererApiPtr = new DX12RenderingApi();
}
