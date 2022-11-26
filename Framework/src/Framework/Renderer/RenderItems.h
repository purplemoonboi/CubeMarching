#pragma once
#include <intsafe.h>

#include "Mesh.h"

#include "Framework/Core/core.h"

namespace Engine
{

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


	constexpr INT32 NUMBER_OF_FRAME_RESOURCES = 1;


	// Lightweight structure stores parameters to draw a shape.  This will
	// vary from app-to-app.
	struct RenderItem
	{
		RenderItem() = default;
		virtual ~RenderItem() = default;

		static ScopePointer<RenderItem> Create(MeshGeometry* geometry, std::string&& drawArgs, UINT constantBufferIndex, Transform transform);

		// Dirty flag indicating the object data has changed and we need to update the constant buffer.
		// Because we have an object cbuffer for each FrameResource, we have to apply the
		// update to each FrameResource.  Thus, when we modify obect data we should set 
		// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
		int NumFramesDirty = NUMBER_OF_FRAME_RESOURCES;

		// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
		UINT ObjectConstantBufferIndex = -1;

		// A raw pointer to geometry
		MeshGeometry* Geometry;

		// DrawIndexedInstanced parameters.
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT32 BaseVertexLocation = 0;
		std::string args = "";
		bool HasBeenUpdated = false;
	};




}
