#pragma once
#include "Buffer.h"
#include "Framework/Core/core.h"


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


	struct Geometry
	{
	

		Geometry(const std::string& name)
			:
			Name(name),
			VertexBuffer(nullptr),
			IndexBuffer(nullptr)
		{}

		Geometry(std::string&& name)
			:
			Name(std::move(name)),
			VertexBuffer(nullptr),
			IndexBuffer(nullptr)
		{}



		static RefPointer<Geometry> Create(std::string&& name)
		{
			return CreateRef<Geometry>(name);
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

	constexpr INT32 NUM_OF_RESOURCES = 3;

	// Lightweight structure stores parameters to draw a shape.  This will
	// vary from app-to-app.
	struct RenderItem
	{
		RenderItem() = default;
		virtual ~RenderItem() = default;

		// Dirty flag indicating the object data has changed and we need to update the constant buffer.
		// Because we have an object cbuffer for each FrameResource, we have to apply the
		// update to each FrameResource.  Thus, when we modify obect data we should set 
		// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
		int NumFramesDirty = NUM_OF_RESOURCES;

		// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
		UINT ObjectConstantBufferIndex = -1;

		// A unique pointer to geometry
		ScopePointer<Geometry> Geometry;

		// DrawIndexedInstanced parameters.
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT32 BaseVertexLocation = 0;
	};

	class GeometryGenerator
	{
	public:
		struct Vertex
		{
			Vertex() {}
			Vertex(
				const DirectX::XMFLOAT3& p,
				const DirectX::XMFLOAT3& n,
				const DirectX::XMFLOAT3& t,
				const DirectX::XMFLOAT2& uv) :
				Position(p),
				Normal(n),
				TangentU(t),
				TexC(uv) {}
			Vertex(
				float px, float py, float pz,
				float nx, float ny, float nz,
				float tx, float ty, float tz,
				float u, float v) :
				Position(px, py, pz),
				Normal(nx, ny, nz),
				TangentU(tx, ty, tz),
				TexC(u, v) {}

			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT3 TangentU;
			DirectX::XMFLOAT2 TexC;
		};

		struct MeshData
		{
			std::vector<Vertex> Vertices;
			std::vector<UINT32> Indices32;

			std::vector<UINT16>& GetIndices16()
			{
				if (Indices16.empty())
				{
					Indices16.resize(Indices32.size());
					for (SIZE_T i = 0; i < Indices32.size(); ++i)
					{
						Indices16[i] = static_cast<UINT16>(Indices32[i]);
					}
				}

				return Indices16;
			}

		private:
			std::vector<UINT16> Indices16;
		};

		///<summary>
		/// Creates a box centered at the origin with the given dimensions, where each
		/// face has m rows and n columns of vertices.
		///</summary>
		MeshData CreateBox(float width, float height, float depth, UINT32 numSubdivisions);

		///<summary>
		/// Creates a sphere centered at the origin with the given radius.  The
		/// slices and stacks parameters control the degree of tessellation.
		///</summary>
		MeshData CreateSphere(float radius, UINT32 sliceCount, UINT32 stackCount);

		///<summary>
		/// Creates a geosphere centered at the origin with the given radius.  The
		/// depth controls the level of tessellation.
		///</summary>
		MeshData CreateGeosphere(float radius, UINT32 numSubdivisions);

		///<summary>
		/// Creates a cylinder parallel to the y-axis, and centered about the origin.  
		/// The bottom and top radius can vary to form various cone shapes rather than true
		// cylinders.  The slices and stacks parameters control the degree of tessellation.
		///</summary>
		MeshData CreateCylinder(float bottomRadius, float topRadius, float height, UINT32 sliceCount, UINT32 stackCount);

		///<summary>
		/// Creates an mxn grid in the xz-plane with m rows and n columns, centered
		/// at the origin with the specified width and depth.
		///</summary>
		MeshData CreateGrid(float width, float depth, UINT32 m, UINT32 n);

		///<summary>
		/// Creates a quad aligned with the screen.  This is useful for postprocessing and screen effects.
		///</summary>
		MeshData CreateQuad(float x, float y, float w, float h, float depth);

	private:
		void Subdivide(MeshData& meshData);
		Vertex MidPoint(const Vertex& v0, const Vertex& v1);
		void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT32 sliceCount, UINT32 stackCount, MeshData& meshData);
		void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT32 sliceCount, UINT32 stackCount, MeshData& meshData);
	};



}
