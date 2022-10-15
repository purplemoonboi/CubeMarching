#include "Framework/cmpch.h"
#include "RendererAPI.h"

namespace DX12Framework
{
	//For this project, forcing API to be DX12
	RendererAPI::API RendererAPI::RendererAPIPtr = RendererAPI::API::DX12;
}