#pragma once
#include <DirectXCollision.h>
#include <string>
#include <unordered_map>
#include "Framework/Renderer/Buffers/Buffer.h"

namespace Engine
{
	// Defines a sub-range of geometry in a MeshGeometry.  This is for when multiple
	// geometries are stored in one vertex and index buffer.  It provides the offsets
	// and data needed to draw a subset of geometry stores in the vertex and index 
	// buffers so that we can implement the technique described by Figure 6.3.
	struct SubGeometry
	{
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT BaseVertexLocation = 0;

		// Bounding box of the geometry defined by this submesh. 
		// This is used in later chapters of the book.
		//DirectX::BoundingBox Bounds;
	};


	struct MeshGeometry
	{



		MeshGeometry(const std::string& name)
			:
			Name(name)

		{}

		MeshGeometry(std::string&& name)
			:
			Name(std::move(name))
		{}


		static ScopePointer<MeshGeometry> Create(std::string&& name)
		{
			return CreateScope<MeshGeometry>(name);
		}

		// @brief Returns the name of the mesh
		const std::string& GetName() const { return Name; }

		// @brief Contains the vertex data for drawing the mesh
		RefPointer<VertexBuffer> VertexBuffer;

		// @brief Contains the indices for drawing the mesh
		RefPointer<IndexBuffer> IndexBuffer;

		// A MeshGeometry may store multiple geometries in one vertex/index buffer.
		// Use this container to define the Submesh geometries so we can draw
		// the Submeshes individually.
		std::unordered_map<std::string, SubGeometry> DrawArgs;

	private:
		// Give it a name so we can look it up by name.
		std::string Name;
	};
}
