#pragma once
#include <intsafe.h>
#include <string>
#include "Framework/Camera/MainCamera.h";
#include "Framework/Core/core.h"
#include "Framework/Core/Time/AppTimeManager.h"
#include "Framework/Renderer/Api/FrameResource.h"

#include <entt.hpp>

namespace entt
{
	registry;
}

namespace Foundation
{
	class Scene;
}

namespace Foundation::Graphics
{

	struct RenderItem;
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

		// @brief - Marks the start of rendering a scene.
		//		
		static void BeginScene(MainCamera* camera, AppTimeManager* time);

		// @brief - Marks the end to capturing rendering instructions.
		//			Calls a flush() once the current block of data is
		//			calculated for rendering.
		static void EndScene();

 		// @brief - Creates the core engine materials. 
		static void BuildCoreEngineDefaultMaterials();

		// @brief - Creates the core engine textures.
		static void BuildCoreEngineDefaultTextures();

		static void CreateCube(float x, float y, float z, std::string& name, UINT32 subDivisions = 1);

		static void CreatePlane(float x, float y, float z, float w, float h, float depth, std::string& name, UINT32 subSivisions = 1);

		static void CreateSphere(float radius,  std::string& name, UINT32 lateralResolution = 6, UINT32 longitudeResolution = 6);

		static void CreateMesh(const std::string& meshTag, Transform transform, INT8 staticMeshType);

		static RenderItem* GetRenderItem(UINT16 index);
	};


}
