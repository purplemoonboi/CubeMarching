#pragma once
#include "Framework/Renderer/Renderer3D/Mesh.h"
#include "Framework/Renderer/Material/Material.h"
#include "Framework/Core/core.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Constants/D3D12GlobalConstants.h"

#include <intsafe.h>
#include <DirectXCollision.h>

namespace Foundation::Graphics
{
	struct Vertex;
	class Texture;

	struct Transform
	{

		Transform
		(
			float X, float Y, float Z, 
			float RX = 0.0f, float RY = 0.0f, float RZ = 0.0f, 
			float SX = 1.0f, float SY = 1.0f, float SZ = 1.0f
		)
			:
			X(X),   Y(Y),   Z(Z),
			RX(RX), RY(RY), RZ(RZ),
			SX(SX), SY(SY), SZ(SZ)
		{}

		float X, Y, Z;
		float RX, RY, RZ;
		float SX, SY, SZ;
	};


	// Lightweight structure stores parameters to draw a shape.  This will
	// vary from app-to-app.
	struct RenderItem
	{
		RenderItem() = default;
		virtual ~RenderItem() = default;

		static ScopePointer<RenderItem> Create(MeshGeometry* geometry, Material* material, const std::string& drawArgs, UINT constantBufferIndex, Transform transform);

		INT32 NumFramesDirty = D3D12::FRAMES_IN_FLIGHT;
		UINT ObjectConstantBufferIndex = -1;

		// A raw pointer to geometry
		MeshGeometry* Geometry;
		Material* Material = nullptr;

		DirectX::BoundingBox Bounds;

		// DrawIndexedInstanced parameters.
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT32 BaseVertexLocation = 0;
		std::string args = "";
		bool HasBeenUpdated = false;
	};


}
