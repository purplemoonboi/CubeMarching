#pragma once
#include "Framework/Core/core.h"
#include "Framework/Renderer/Renderer3D/Mesh.h"
#include "Framework/Renderer/Material/Material.h"
#include "Framework/Renderer/Settings/Settings.h"



namespace Foundation::Graphics
{
	struct Vertex;
	class Texture;

	// Lightweight structure stores parameters to draw a shape.  This will
	// vary from app-to-app.
	struct RenderItem
	{
		RenderItem() = default;
		virtual ~RenderItem() = default;

		static ScopePointer<RenderItem> Create(MeshGeometry* geometry, Material* material, const std::string& drawArgs, UINT constantBufferIndex);

		INT32 NumFramesDirty = FRAMES_IN_FLIGHT;
		UINT ObjectConstantBufferIndex = -1;

		// A raw pointer to geometry
		MeshGeometry* Geometry;
		Material* Material = nullptr;


		// DrawIndexedInstanced parameters.
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT32 BaseVertexLocation = 0;
		std::string args = "";
		bool HasBeenUpdated = false;
	};


}
