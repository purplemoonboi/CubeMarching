#pragma once
#include "Framework/Renderer/Resources/RenderItems.h"

namespace Engine
{
	class D3D12TerrainItem : public TerrainItem
	{
	public:

		ID3D12Resource* VertexBuffer;

	};
}
