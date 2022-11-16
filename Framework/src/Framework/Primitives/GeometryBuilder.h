#pragma once
#include "Platform/DirectX12/DirectX12.h"
#include <cstdint>
#include <DirectXMath.h>
#include <vector>

namespace Engine
{
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