#pragma once
#include <DirectXMath.h>
#include <intsafe.h>
#include <vector>



	/**
	 * Please see below the class for constant data tables.
	 */

	using namespace DirectX;

	struct Triangle
	{
		XMFLOAT3 Vertex[3];
		XMFLOAT3 Normals[3];
	};

	struct Voxel
	{
		XMFLOAT3 Position[8]; // 32 * 3 bytes
		float Padding = 0.0f; // 32 bytes
		double Value[8]; // 64 bytes
	};

	class MarchingCubesSolver
	{

	public:

		// @brief - Default constructor
		MarchingCubesSolver() = default;
		// @brief - Default destructor
		virtual ~MarchingCubesSolver() = default;

		// @brief - Copy constructor
		MarchingCubesSolver(const MarchingCubesSolver& marchingCubes) noexcept;
		// @brief - Copy assignment
		auto operator=(const MarchingCubesSolver& marchingCubes) noexcept -> MarchingCubesSolver&;
		// @brief - Move constructor
		MarchingCubesSolver(MarchingCubesSolver&& marchingCubes) noexcept;
		// @brief - Move constructor
		auto operator=(MarchingCubesSolver&& marchingCubes) noexcept -> MarchingCubesSolver&&;

		// @brief - Invokes classic marching cubes algorithm on a given 3 - dimensional cubic lattice.
		INT32 PolygoniseScalarField(Voxel voxel, double isoLevel, std::vector<Triangle>* triangles);

		// @brief - Linear interpolate between two points
		XMFLOAT3 VertexLinearLerp(double isoLevel, XMFLOAT3 point0, XMFLOAT3 point1, double value0, double value1);

	protected:
	};




