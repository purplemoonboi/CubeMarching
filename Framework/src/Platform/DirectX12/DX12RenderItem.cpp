#include "DX12RenderItem.h"
#include "Framework/Renderer/Buffer.h"

namespace Engine
{
	DX12RenderItem::DX12RenderItem
	(
		MeshGeometry* geometry,
		std::string&& drawArgs,
		UINT constantBufferIndex,
		Transform transform
	)
	{
		World = MathHelper::Identity4x4();
		DirectX::XMStoreFloat4x4
		(
			&World,
			DirectX::XMMatrixScaling(transform.SX, transform.SY, transform.SZ) * DirectX::XMMatrixTranslation(transform.X, transform.Y, transform.Z)
		);

		ObjectConstantBufferIndex = constantBufferIndex;

		Geometry = geometry;
		PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		IndexCount = Geometry->DrawArgs[drawArgs].IndexCount;
		//IndexCount = Geometry->IBuffer->GetCount();
		StartIndexLocation = Geometry->DrawArgs[drawArgs].StartIndexLocation;
		BaseVertexLocation = Geometry->DrawArgs[drawArgs].BaseVertexLocation;

		args = drawArgs;


	}
}
