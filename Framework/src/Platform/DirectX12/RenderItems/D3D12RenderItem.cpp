#include "D3D12RenderItem.h"
#include "Framework/Renderer/Buffers/Buffer.h"

namespace Engine
{
	D3D12RenderItem::D3D12RenderItem
	(
		MeshGeometry* geometry,
		Engine::Material* material,
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
		Material = material;
		PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		IndexCount = Geometry->DrawArgs[drawArgs].IndexCount;
		//IndexCount = Geometry->IBuffer->GetCount();
		StartIndexLocation = Geometry->DrawArgs[drawArgs].StartIndexLocation;
		BaseVertexLocation = Geometry->DrawArgs[drawArgs].BaseVertexLocation;

		args = drawArgs;


	}
}
