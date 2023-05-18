#include "Framework/cmpch.h"
#include "RendererAPI.h"

namespace Foundation
{
	//For this project, forcing Api to be DX12
	RendererAPI::Api RendererAPI::RenderingApi = RendererAPI::Api::DX12;
}