#pragma once
#include "DirectX12.h"
#include "MathHelper.h"
#include "Framework/Renderer/Resources/RenderItems.h"

namespace Engine
{
	using Microsoft::WRL::ComPtr;


	//Implemented render item for the DX12 api
	struct DX12RenderItem : RenderItem
	{
		DX12RenderItem
		(
			MeshGeometry* geometry,
			Engine::Material* material,
			std::string&& drawArgs,
			UINT constantBufferIndex,
			Transform transform
		);

		~DX12RenderItem() override = default;

		// World matrix of the shape that describes the object's local space
		// relative to the world space, which defines the position, orientation,
		// and scale of the object in the world.
		DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();

		// Primitive topology.
		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	};

}
