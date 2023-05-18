#include "Framework/cmpch.h"
#include "ComputeInstruction.h"

#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"
namespace Foundation
{
	ComputeApi* ComputeInstruction::Compute = new D3D12ComputeApi();
}
