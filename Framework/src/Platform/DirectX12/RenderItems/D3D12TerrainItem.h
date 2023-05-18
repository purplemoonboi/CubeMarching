#pragma once
#include "Framework/Renderer/Resources/RenderItems.h"

namespace Foundation
{
	class D3D12TerrainItem : public TerrainItem
	{
	public:

		ID3D12Resource* VertexBuffer;

	};
}
