#include "Framework/cmpch.h"
#include "RenderInstruction.h"

#include "Platform/DirectX12/DX12RenderingApi.h"

namespace DX12Framework
{
	RendererAPI* RenderInstruction::RendererApiPtr = new DX12RenderingApi();
}
