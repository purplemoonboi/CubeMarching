#pragma once
#include <intsafe.h>
#include <string>
#include "GeometryGenerator.h"
#include "Framework/Camera/MainCamera.h";
#include "Framework/Core/core.h"
#include "Framework/Renderer/Api/FrameResource.h"


namespace Engine
{
	struct WorldSettings;
	struct Transform;
	struct MeshGeometry;
	struct MCTriangle;

	class RendererApi;
	class GraphicsContext;


	class Renderer3D
	{
	public:

		enum class RenderLayer
		{
			Opaque = 0,
			Transparent
		};

		// @brief - Resets command lists for Apis such as DirectX12
		static void PreInit();

		// @brief - Initialises descriptions of vertex buffer.
		static void Init();

		// @brief - Closes command lists for Apis such as DirectX12 
		static void PostInit();

		// @brief - Cleans the rendering system.
		static void Shutdown();

		// @brief - Marks the start of rendering commands.
		//		
		static void BeginScene(const MainCamera& cam, const WorldSettings& settings, const float deltaTime, bool wireframe = false, const float elapsedTime = 0.0f);

		// @brief - Marks the end to capturing rendering instructions.
		//			Calls a flush() once the current block of data is
		//			calculated for rendering.
		static void EndScene();

		static void BuildMaterials();

		static void BuildTextures();

		static void CreateCube(float x, float y, float z, std::string& name, UINT32 subDivisions = 1);

		static void CreatePlane(float x, float y, float z, float w, float h, float depth, std::string& name, UINT32 subSivisions = 1);

		static void CreateSphere(float radius,  std::string& name, UINT32 lateralResolution = 6, UINT32 longitudeResolution = 6);

		static void CreateVoxelTerrain(std::vector<Vertex> vertices, std::vector<UINT16> indices, const std::string& meshTag, Transform transform);

		static void CreateMesh(const std::string& meshTag, Transform transform, INT8 staticMeshType);

		static void SetBuffer(const std::string& renderItemTag, const std::vector<Vertex>& vertices, const std::vector<UINT16>& indices);
		static void SetTerrainBuffer(const std::vector<Vertex>& vertices, const std::vector<UINT16>& indices);

		static RenderItem* GetRenderItem(UINT16 index);

		struct VoxelWorldRenderingStats
		{
			UINT32 MCDrawCalls = 0;
			UINT32 MCTriCount = 0;
			UINT32 MCPolyCount = 0;

			UINT32 DCDrawCalls = 0;
			UINT32 DCTriCount = 0;
			UINT32 DCPolyCount = 0;

			//TODO: Implement this properly!
			UINT32 GetTotalVertexCount() { return 0; }
			//TODO: Implement this properly!
			UINT32 GetTotalIndexCount() { return 0; }
		};

		static const VoxelWorldRenderingStats& GetProfileData() { return VoxelStats; }

		static VoxelWorldRenderingStats VoxelStats;


	};


}
