#include "Framework/cmpch.h"
#include "RenderInstruction.h"

#include "Platform/DirectX12/Api/D3D12RenderingApi.h"

namespace Engine
{
	ScopePointer<RendererAPI> RenderInstruction::RendererApiPtr = CreateScope<D3D12RenderingApi>(0);
}
