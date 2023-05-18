#pragma once
#include "Platform/DirectX12/DirectX12.h"
#include "MathHelper.h"
#include "Framework/Renderer/Resources/RenderItems.h"

namespace Foundation
{
	using Microsoft::WRL::ComPtr;


	//Implemented render item for the DX12 api
	struct D3D12RenderItem : RenderItem
	{
		D3D12RenderItem
		(
			MeshGeometry* geometry,
			Foundation::Material* material,
			const std::string& drawArgs,
			UINT constantBufferIndex,
			Transform transform
		);

		~D3D12RenderItem() override = default;

		// Scene matrix of the shape that describes the object's local space
		// relative to the world space, which defines the position, orientation,
		// and scale of the object in the world.
		DirectX::XMFLOAT4X4 World			= MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 TexTransforms	= MathHelper::Identity4x4();

		// Primitive topology.
		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	};

}
