#include "Renderer3D.h"


#include "GeometryGenerator.h"
#include "RenderInstruction.h"


#include <DirectXColors.h>

#include "Framework/Core/Compute/ComputeInstruction.h"
#include "Framework/Renderer/Api/FrameResource.h"
#include "Framework/Renderer/Buffers/VertexArray.h"
#include "Framework/Renderer/Pipeline/PipelineStateObject.h"
#include "Framework/Renderer/Resources/RenderItems.h"
#include "Framework/Renderer/Resources/Shader.h"
#include "Framework/Renderer/Resources/Material.h"
#include "Framework/Renderer/Textures/Texture.h"
#include "IsoSurface/DensityTextureGenerator.h"

#include "IsoSurface/MarchingCubes.h"

namespace Engine
{

	Renderer3D::VoxelWorldRenderingStats Renderer3D::VoxelStats;
    GeometryGenerator Geo;

	struct RendererData
	{
		// Constant data
		static constexpr UINT32 MaxTriangles = 100000;
		static constexpr UINT32 MaxVertices = MaxTriangles * 4;
		static constexpr UINT32 MaxIndices = MaxTriangles * 6;
		static constexpr UINT32 MaxTextureSlots = 32;

		ScopePointer<VertexArray> VertexArray;

		ScopePointer<Shader> VertexShader;
		ScopePointer<Shader> PixelShader;

		ShaderLibrary ShaderLibrary;
		MaterialLibrary MaterialLibrary;
		TextureLibrary TextureLibrary;

		std::vector<Material*> Materials;
		std::vector<Texture*> Textures;

		std::unordered_map<std::string, ScopePointer<MeshGeometry>> Geometries;


		std::vector<ScopePointer<RenderItem>> RenderItems;
		std::vector<RenderItem*> OpaqueRenderItems;
		std::vector<RenderItem*> WireFrameRenderItems;


		std::unordered_map<std::string, RefPointer<PipelineStateObject>> PSOs;

		UINT64 BaseVertexLocation = 0;

		//Renderer3D::RenderingStats RendererStats;

	};

	static RendererData RenderData;


	void Renderer3D::PreInit()
	{
		RenderInstruction::PreInitRenderer();
	}

	void Renderer3D::PostInit()
	{
		RenderInstruction::PostInitRenderer();
	}

	void Renderer3D::Init()
	{
		const auto api = RenderInstruction::GetApiPtr();

		/** build and compile shaders */
		auto vS = Shader::Create(L"assets\\shaders\\Default.hlsl", "VS", "vs_5_1");
		auto pS = Shader::Create(L"assets\\shaders\\Default.hlsl", "PS", "ps_5_1");

		RenderData.ShaderLibrary.Add("vs", std::move(vS));
		RenderData.ShaderLibrary.Add("ps", std::move(pS));

		const BufferLayout layout =
		{
			{"POSITION",	ShaderDataType::Float3, 0,  0},
			{"NORMAL",		ShaderDataType::Float3, 12, 1},
			{"TEXCOORD",	ShaderDataType::Float2, 24, 2},
		};

		BuildTextures();
		BuildMaterials();

		/** build the pipeline state objects */
		RenderData.PSOs.emplace("Opaque", PipelineStateObject::Create
		(
			api->GetGraphicsContext(),
			RenderData.ShaderLibrary.GetShader("vs"),
			RenderData.ShaderLibrary.GetShader("ps"),
			layout,
			FillMode::Opaque
		));

		RenderData.PSOs.emplace("Wire", PipelineStateObject::Create
		(
			api->GetGraphicsContext(),
			RenderData.ShaderLibrary.GetShader("vs"),
			RenderData.ShaderLibrary.GetShader("ps"),
			layout,
			FillMode::WireFrame
		));

		auto gridData = Geo.CreateGrid(100, 100, 12, 12);
		auto gridMesh = MeshGeometry::Create("EditorGrid");
		
		gridMesh->VertexBuffer = VertexBuffer::Create(
			gridData.Vertices.data(),
			sizeof(Vertex) * gridData.Vertices.size(),
			gridData.Vertices.size(),
			false);

		gridMesh->IndexBuffer = IndexBuffer::Create(
			gridData.GetIndices16().data(),
			sizeof(UINT16) * gridData.GetIndices16().size(),
			gridData.GetIndices16().size());

		SubGeometry geoArgs = {(UINT)gridMesh->IndexBuffer->GetCount(), 0, 0};
		gridMesh->DrawArgs.emplace("EditorArgs", geoArgs);

		RenderData.Geometries.emplace("EditorGrid", std::move(gridMesh));

		ScopePointer<RenderItem> renderItem = RenderItem::Create
		(
			RenderData.Geometries["EditorGrid"].get(),
			RenderData.MaterialLibrary.Get("Default"),
			"EditorArgs",
			0,
			Transform(0,0,0)
		);

		RenderData.OpaqueRenderItems.push_back(renderItem.get());
		RenderData.RenderItems.push_back(std::move(renderItem));



		auto boxData = Geo.CreateBox(1, 1, 1, 1);
		auto boxMesh = MeshGeometry::Create("Box");
		boxMesh->VertexBuffer = VertexBuffer::Create(
			boxData.Vertices.data(),
			sizeof(Vertex) * boxData.Vertices.size(),
			boxData.Vertices.size(),
			false);

		boxMesh->IndexBuffer = IndexBuffer::Create(
			boxData.GetIndices16().data(),
			sizeof(UINT16) * boxData.GetIndices16().size(),
			boxData.GetIndices16().size());

		SubGeometry boxArgs = { (UINT)boxMesh->IndexBuffer->GetCount(), 0, 0 };
		boxMesh->DrawArgs.emplace("Box", boxArgs);

		RenderData.Geometries.emplace("Box", std::move(boxMesh));

		ScopePointer<RenderItem> boxItem = RenderItem::Create
		(
			RenderData.Geometries["Box"].get(),
			RenderData.MaterialLibrary.Get("Green"),
			"BoxArgs",
			1,
			Transform(-5, 5, 5, 0, 45, 0, 10, 10, 10)
		);

		
		RenderData.OpaqueRenderItems.push_back(boxItem.get());
		RenderData.RenderItems.push_back(std::move(boxItem));

		

	}

	void Renderer3D::Shutdown()
	{}

	void Renderer3D::BeginScene(const MainCamera& cam, const WorldSettings& settings, const float deltaTime, bool wireframe, const float elapsedTime)
	{
		
		/*
		 * stage data for rendering
		 * here we execute any copies for apis which rely on manual control of syncing data
		 */
		RenderInstruction::PreRender
		(
			RenderData.OpaqueRenderItems,
			RenderData.Materials,
			cam,
			settings,
			deltaTime,
			elapsedTime,
			wireframe
		);


		auto* pso = (wireframe) ? RenderData.PSOs["Wire"].get() : RenderData.PSOs["Opaque"].get();


		RenderInstruction::BindGeometryPass(pso, RenderData.OpaqueRenderItems);
	}

	void Renderer3D::EndScene()
	{
		/**
		 *	Render the scene geometry to the scene
		 */
		
		//ProfileStats.MCDrawCalls = 1;
		RenderInstruction::Flush();
		RenderInstruction::PostRender();
		
	}

	void Renderer3D::BuildMaterials()
	{
		auto mat = Material::Create("Green");
		mat->SetDiffuse(0.0f, 1.0f, 0.3f, 1.0f);
		mat->SetFresnel(0.05f, 0.05f, 0.05f);
		mat->SetRoughness(0.2f);
		
		mat->SetMaterialBufferIndex(0);
		RenderData.MaterialLibrary.Add("Green", std::move(mat));
		RenderData.Materials.push_back(RenderData.MaterialLibrary.Get("Green"));

		auto matB = Material::Create("Default");
		matB->SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
		matB->SetFresnel(0.02f, 0.02f, 0.02f);
		matB->SetRoughness(0.5f);
		matB->SetMaterialBufferIndex(1);
		RenderData.MaterialLibrary.Add("Default", std::move(matB));
		RenderData.Materials.push_back(RenderData.MaterialLibrary.Get("Default"));

		auto terrainMaterial = Material::Create("Terrain");
		terrainMaterial->SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
		terrainMaterial->SetFresnel(0.02f, 0.02f, 0.02f);
		terrainMaterial->SetRoughness(0.5f);
		terrainMaterial->SetMaterialBufferIndex(1);
		// Terrain get sepcial material setup
		//for(INT32 i =1; i<7;++i)
		//{
		//	terrainMaterial->Textures[(i-1)] = RenderData.Textures[i];
		//}

		RenderData.MaterialLibrary.Add("Terrain", std::move(terrainMaterial));
		RenderData.Materials.push_back(RenderData.MaterialLibrary.Get("Terrain"));
	}

	void Renderer3D::BuildTextures()
	{
		/*auto texture = Texture::Create(L"assets\\textures\\crate\\WoodCrate02.dds", "Crate");
		RenderData.Textures.push_back(texture.get());
		RenderData.TextureLibrary.Add("Crate", std::move(texture));*/


		/**
		 *	moss
		 */

		//auto mossAlbedo = Texture::Create(L"assets\\textures\\moss\\Moss_albedo.dds", "MossAlbedo");
		//RenderData.Textures.push_back(mossAlbedo.get());
		//RenderData.TextureLibrary.Add("MossAlbedo", std::move(mossAlbedo));

		//auto mossNormal = Texture::Create(L"assets\\textures\\moss\\Moss_normal.dds", "MossNormal");
		//RenderData.Textures.push_back(mossNormal.get());
		//RenderData.TextureLibrary.Add("MossNormal", std::move(mossNormal));

		//auto mossRough = Texture::Create(L"assets\\textures\\moss\\Moss_roughness.dds", "MossRough");
		//RenderData.Textures.push_back(mossRough.get());
		//RenderData.TextureLibrary.Add("MossRough", std::move(mossRough));


		///**
		// *  rock
		// */


		//auto rockAlbedo = Texture::Create(L"assets\\textures\\rock\\RocksLayered02_albedo.dds", "RockAlbedo");
		//RenderData.Textures.push_back(rockAlbedo.get());
		//RenderData.TextureLibrary.Add("RockAlbedo", std::move(rockAlbedo));

		//auto rockNormal = Texture::Create(L"assets\\textures\\rock\\RocksLayered02_normal.dds", "RockNormal");
		//RenderData.Textures.push_back(rockNormal.get());
		//RenderData.TextureLibrary.Add("RockNormal", std::move(rockNormal));

		//auto rockRough = Texture::Create(L"assets\\textures\\rock\\RocksLayered02_roughness.dds", "RockRough");
		//RenderData.Textures.push_back(rockRough.get());
		//RenderData.TextureLibrary.Add("RockRough", std::move(rockRough));
	}

	void Renderer3D::RegenerateBuffers
	(
		const std::string& meshTag,
		std::vector<Vertex> vertices,
		std::vector<UINT16> indices
	)
	{

		if (RenderData.Geometries.find(meshTag) != RenderData.Geometries.end())
		{
			VertexBuffer* vertexBuffer = RenderData.Geometries[meshTag]->VertexBuffer.get();
			IndexBuffer* indexBuffer = RenderData.Geometries[meshTag]->IndexBuffer.get();

			//vertexBuffer->Destroy();
			//indexBuffer->Destroy();

			const INT32 size = vertices.size() * sizeof(Vertex);
			vertexBuffer->SetData(vertices.data(), size, vertices.size());
			indexBuffer->SetData(indices.data(), indices.size());

			if(meshTag=="MarchingTerrain")
			{
				VoxelStats.MCPolyCount = vertices.size();
				VoxelStats.MCTriCount = vertices.size()/3;
			}
			if(meshTag=="DualTerrain")
			{
				VoxelStats.DCPolyCount = vertices.size();
				VoxelStats.DCTriCount = vertices.size() / 3;
			}

			// We need to schedule the creation of the buffers
			// as this does not happen immediately for some
			// apis. 
			RenderData.Geometries[meshTag]->DirtFlag = 1;
		}

	}

	void Renderer3D::CreateVoxelMesh
	(
		std::vector<Vertex> vertices,
		std::vector<UINT16> indices,
		const std::string& meshTag,
		Transform transform
	)
	{
		if (RenderData.OpaqueRenderItems.size() < 20)
		{


			if (RenderData.Geometries.find(meshTag) == RenderData.Geometries.end())
			{

				ScopePointer<MeshGeometry> mesh = CreateScope<MeshGeometry>(meshTag);

				mesh->VertexBuffer = VertexBuffer::Create(
					vertices.data(),
					sizeof(Vertex) * vertices.size(),
					vertices.size(),
					false);

				mesh->IndexBuffer = IndexBuffer::Create(
					indices.data(),
					sizeof(UINT16) * indices.size(),
					indices.size());

				SubGeometry drawArgs = { (UINT)mesh->IndexBuffer->GetCount(), 0, 0 };
				mesh->DrawArgs.emplace(meshTag + "_Args", drawArgs);

				RenderData.Geometries.emplace(meshTag, std::move(mesh));

				const INT32 constCbvOffset = RenderData.OpaqueRenderItems.size();

				ScopePointer<RenderItem> renderItem = RenderItem::Create
				(
					RenderData.Geometries[meshTag].get(),
					RenderData.MaterialLibrary.Get("Terrain"),
					meshTag+"_Args",
					constCbvOffset,
					transform
				);

				


				RenderData.OpaqueRenderItems.push_back(renderItem.get());
				RenderData.RenderItems.push_back(std::move(renderItem));
			}
		}
			
		
	}



	void Renderer3D::CreateCube(float x, float y, float z, std::string& name, UINT32 subDivisions)
	{
		auto api = RenderInstruction::GetApiPtr();
		auto cubeData = Geo.CreateBox(x, y, z, subDivisions);

		ScopePointer<MeshGeometry> cubeGeometry = CreateScope<MeshGeometry>(name);
		const UINT vbSizeInBytes = sizeof(Vertex) * cubeData.Vertices.size();
		const UINT ibSizeInBytes = sizeof(UINT16) * cubeData.GetIndices16().size();

		cubeGeometry->VertexBuffer = VertexBuffer::Create(
			cubeData.Vertices.data(), vbSizeInBytes, cubeData.Vertices.size(), false);

		BufferLayout layout = BufferLayout
		({ 
			{ "VERTEX",	  ShaderDataType::Float3, 0,   0 },
			{ "NORMAL",	  ShaderDataType::Float3, 12,  1 },
			{ "TEXCOORDS", ShaderDataType::Float2, 24, 2 } 
		});
		
		cubeGeometry->VertexBuffer->SetLayout(layout);

		cubeGeometry->IndexBuffer = IndexBuffer::Create(
			cubeData.GetIndices16().data(), ibSizeInBytes, cubeData.GetIndices16().size());

		if (RenderData.Geometries.find(name) == RenderData.Geometries.end())
		{
			RenderData.Geometries.emplace(name, std::move(cubeGeometry));
		}
		else
		{
			name.append("_0");
			RenderData.Geometries.emplace(name, std::move(cubeGeometry));
		}
	}

	void Renderer3D::CreatePlane(float x, float y, float z, float w, float h, float depth, std::string& name,  UINT32 subSivisions)
	{
		auto api = RenderInstruction::GetApiPtr();
		auto planeData = Geo.CreateQuad(x, y, z, w, h);

		ScopePointer<MeshGeometry> planeGeometry = CreateScope<MeshGeometry>(name);
		const UINT vbSizeInBytes = sizeof(Vertex) * planeData.Vertices.size();
		const UINT ibSizeInBytes = sizeof(UINT16) * planeData.GetIndices16().size();

		planeGeometry->VertexBuffer = VertexBuffer::Create(
			planeData.Vertices.data(), vbSizeInBytes, planeData.Vertices.size(), false);

		planeGeometry->IndexBuffer = IndexBuffer::Create(
			planeData.GetIndices16().data(), ibSizeInBytes, planeData.GetIndices16().size());

		BufferLayout layout = BufferLayout
		({
			{ "VERTEX",	  ShaderDataType::Float3, 0,   0 },
			{ "NORMAL",	  ShaderDataType::Float3, 12,  1 },
			{ "TEXCOORDS", ShaderDataType::Float2, 24, 2 }
			});

		planeGeometry->VertexBuffer->SetLayout(layout);

		if (RenderData.Geometries.find(name) == RenderData.Geometries.end())
		{
			RenderData.Geometries.emplace(name, std::move(planeGeometry));
		}
		else
		{
			name.append("_0");
			RenderData.Geometries.emplace(name, std::move(planeGeometry));
		}
	}

	void Renderer3D::CreateSphere(float radius, std::string& name, UINT32 lateralResolution, UINT32 longitudeResolution)
	{
		auto api = RenderInstruction::GetApiPtr();
		auto sphereData = Geo.CreateSphere(radius, lateralResolution, longitudeResolution);

		ScopePointer<MeshGeometry> sphereGeometry = CreateScope<MeshGeometry>(name);
		const UINT vbSizeInBytes = sizeof(Vertex) * sphereData.Vertices.size();
		const UINT ibSizeInBytes = sizeof(UINT16) * sphereData.GetIndices16().size();

		sphereGeometry->VertexBuffer = VertexBuffer::Create(
			sphereData.Vertices.data(), vbSizeInBytes, sphereData.Vertices.size(), false);

		sphereGeometry->IndexBuffer = IndexBuffer::Create(
			sphereData.GetIndices16().data(), ibSizeInBytes, sphereData.GetIndices16().size());

		BufferLayout layout = BufferLayout
		({
			{ "VERTEX",	  ShaderDataType::Float3, 0,   0 },
			{ "NORMAL",	  ShaderDataType::Float3, 12,  1 },
			{ "TEXCOORDS", ShaderDataType::Float2, 24, 2 }
			});

		sphereGeometry->VertexBuffer->SetLayout(layout);

		if (RenderData.Geometries.find(name) == RenderData.Geometries.end())
		{
			RenderData.Geometries.emplace(name, std::move(sphereGeometry));
		}
		else
		{
			name.append("_0");
			RenderData.Geometries.emplace(name, std::move(sphereGeometry));
		}
	}

}
