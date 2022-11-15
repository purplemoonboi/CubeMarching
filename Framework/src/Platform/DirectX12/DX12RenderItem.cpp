#include "DX12RenderItem.h"
#include "Framework/Renderer/Buffer.h"

namespace Engine
{
	DX12RenderItem::DX12RenderItem
	(
		const RefPointer<MeshGeometry> geometry,
		std::string&& drawArgs,
		UINT constantBufferIndex,
		Transform&& transform
	)
	{

		DirectX::XMStoreFloat4x4
		(
			&World,
			DirectX::XMMatrixScaling(transform.SX, transform.SY, transform.SZ) *
			DirectX::XMMatrixTranslation(transform.X, transform.Y, transform.Z)
		);

		ObjectConstantBufferIndex = constantBufferIndex;

		Geometry = geometry;
		PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		IndexCount = geometry->IBuffer->GetCount();
		StartIndexLocation = geometry->DrawArgs[drawArgs].StartIndexLocation;
		BaseVertexLocation = geometry->DrawArgs[drawArgs].BaseVertexLocation;


	}
}
