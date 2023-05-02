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

		ScopePointer<RenderItem> Terrain;

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

		D3D_SHADER_MACRO alphaTestDefines[] =
		{
			"ALPHA_TEST", "1",
			NULL, NULL
		};

		/** build and compile shaders */
		auto vS  = Shader::Create(L"assets\\shaders\\Default.hlsl", "VS", "vs_5_1");
		auto pS  = Shader::Create(L"assets\\shaders\\Default.hlsl", "PS", "ps_5_1");
		auto pTS = Shader::Create(L"assets\\shaders\\TerrainPS.hlsl", "PS", "ps_5_1");

		auto vSh = Shader::Create(L"assets\\shaders\\Shadows.hlsl", "VS", "vs_5_1");
		auto pSh = Shader::Create(L"assets\\shaders\\Shadows.hlsl", "PS", "ps_5_1");
		auto pShAl = Shader::Create(L"assets\\shaders\\Shadows.hlsl", "PS", "ps_5_1", alphaTestDefines);

		auto vSd = Shader::Create(L"assets\\shaders\\ShadowDebug.hlsl","VS", "vs_5_1");
		auto pSd = Shader::Create(L"assets\\shaders\\ShadowDebug.hlsl","PS", "ps_5_1");

		auto vSk = Shader::Create(L"assets\\shaders\\Sky.hlsl","VS", "vs_5_1");
		auto pSk = Shader::Create(L"assets\\shaders\\Sky.hlsl","PS", "ps_5_1");


		RenderData.ShaderLibrary.Add("vDefault", std::move(vS));
		RenderData.ShaderLibrary.Add("pAlbedo", std::move(pS));
		RenderData.ShaderLibrary.Add("pTerrain", std::move(pTS));
		RenderData.ShaderLibrary.Add("vShadow", std::move(vSh));
		RenderData.ShaderLibrary.Add("pShadow", std::move(pSh));
		RenderData.ShaderLibrary.Add("paShadow", std::move(pShAl));
		RenderData.ShaderLibrary.Add("vdShadow", std::move(vSd));
		RenderData.ShaderLibrary.Add("pdShadow", std::move(pSd));
		RenderData.ShaderLibrary.Add("vSky", std::move(vSk));
		RenderData.ShaderLibrary.Add("pSky", std::move(pSk));

		const BufferLayout layout =
		{
			{"POSITION",	ShaderDataType::Float3, 0,  0},
			{"NORMAL",		ShaderDataType::Float3, 12, 1},
			{"TANGENT",		ShaderDataType::Float3, 24, 1},
			{"TEXCOORD",	ShaderDataType::Float2, 32, 3},
		};

		const BufferLayout shadowLayout =
		{
			{"POSITION",	ShaderDataType::Float3, 0,  0},
			{"TEXCOORD",	ShaderDataType::Float2, 12, 1},
		};

		BuildTextures();
		BuildMaterials();

		/** build the pipeline state objects */
		RenderData.PSOs.emplace("Opaque", PipelineStateObject::CreatePso
		(
			api->GetGraphicsContext(),
			RenderData.ShaderLibrary.GetShader("vDefault"),
			RenderData.ShaderLibrary.GetShader("pAlbedo"),
			layout,
			FillMode::Opaque
		));

		RenderData.PSOs.emplace("Terrain", PipelineStateObject::CreatePso
		(
			api->GetGraphicsContext(),
			RenderData.ShaderLibrary.GetShader("vDefault"),
			RenderData.ShaderLibrary.GetShader("pTerrain"),
			layout,
			FillMode::Opaque
		));

		RenderData.PSOs.emplace("Shadow", PipelineStateObject::CreateShadowPso
		(
			api->GetGraphicsContext(),
			RenderData.ShaderLibrary.GetShader("vShadow"),
			RenderData.ShaderLibrary.GetShader("pShadow"),
			shadowLayout)
		);

		RenderData.PSOs.emplace("ShadowDebug", PipelineStateObject::CreateShadowPso
		(
			api->GetGraphicsContext(),
			RenderData.ShaderLibrary.GetShader("vdShadow"),
			RenderData.ShaderLibrary.GetShader("pdShadow"),
			shadowLayout
		));

		RenderData.PSOs.emplace("Sky", PipelineStateObject::CreateSkyPso
		(
			api->GetGraphicsContext(),
			RenderData.ShaderLibrary.GetShader("vSky"),
			RenderData.ShaderLibrary.GetShader("pSky")
		));

		RenderData.PSOs.emplace("Wire", PipelineStateObject::CreatePso
		(
			api->GetGraphicsContext(),
			RenderData.ShaderLibrary.GetShader("vDefault"),
			RenderData.ShaderLibrary.GetShader("pAlbedo"),
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

	}

	void Renderer3D::Shutdown()
	{}

	void Renderer3D::BeginScene(const MainCamera& cam, WorldSettings& settings, const float deltaTime, bool wireframe, const float elapsedTime)
	{
		
		/*
		 * stage data for rendering
		 * here we execute any copies for apis which rely on manual control of syncing data
		 */
		RenderInstruction::PreRender
		(
			RenderData.OpaqueRenderItems,
			RenderData.Materials,
			RenderData.Terrain.get(),
			cam,
			settings,
			deltaTime,
			elapsedTime,
			wireframe
		);


		auto* pso = (wireframe) ? RenderData.PSOs["Wire"].get() : RenderData.PSOs["Opaque"].get();
		auto* tpso = (wireframe) ? RenderData.PSOs["Wire"].get() : RenderData.PSOs["Terrain"].get();
		auto* spso = RenderData.PSOs["Shadow"].get();

		RenderInstruction::BindShadowPass(spso, RenderData.OpaqueRenderItems);

		//RenderInstruction::BindTerrainPass(tpso, RenderData.Terrain.get());

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
		terrainMaterial->SetDiffuseTexIndex(0);
		terrainMaterial->SetMaterialBufferIndex(2);

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
		auto mossAlbedo = Texture::Create(L"assets\\textures\\moss\\Moss_albedo.dds", "MossAlbedo");
		RenderData.Textures.push_back(mossAlbedo.get());
		RenderData.TextureLibrary.Add("MossAlbedo", std::move(mossAlbedo));

		auto mossNormal = Texture::Create(L"assets\\textures\\moss\\Moss_normal.dds", "MossNormal");
		RenderData.Textures.push_back(mossNormal.get());
		RenderData.TextureLibrary.Add("MossNormal", std::move(mossNormal));

		//auto mossRough = Texture::Create(L"assets\\textures\\moss\\Moss_roughness.dds", "MossRough");
		//RenderData.Textures.push_back(mossRough.get());
		//RenderData.TextureLibrary.Add("MossRough", std::move(mossRough));


		///**
		// *  rock
		// */
		auto rockAlbedo = Texture::Create(L"assets\\textures\\rock\\RocksLayered02_albedo.dds", "RockAlbedo");
		RenderData.Textures.push_back(rockAlbedo.get());
		RenderData.TextureLibrary.Add("RockAlbedo", std::move(rockAlbedo));

		auto rockNormal = Texture::Create(L"assets\\textures\\rock\\RocksLayered02_normal.dds", "RockNormal");
		RenderData.Textures.push_back(rockNormal.get());
		RenderData.TextureLibrary.Add("RockNormal", std::move(rockNormal));

		//auto rockRough = Texture::Create(L"assets\\textures\\rock\\RocksLayered02_roughness.dds", "RockRough");
		//RenderData.Textures.push_back(rockRough.get());
		//RenderData.TextureLibrary.Add("RockRough", std::move(rockRough));
	}

	void Renderer3D::CreateVoxelTerrain
	(
		const std::vector<Vertex>& vertices,
		std::vector<UINT16>& indices,
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
					nullptr,
					sizeof(Vertex) * 1,
					1,
					false);

				mesh->IndexBuffer = IndexBuffer::Create(
					nullptr,
					sizeof(UINT16) * 1,
					0);

				SubGeometry drawArgs = { (UINT)mesh->IndexBuffer->GetCount(), 0, 0 };
				mesh->DrawArgs.emplace(meshTag + "_Args", drawArgs);

				RenderData.Geometries.emplace(meshTag, std::move(mesh));

				const INT32 constCbvOffset = RenderData.OpaqueRenderItems.size();

				RenderData.Terrain = RenderItem::Create
				(
					RenderData.Geometries[meshTag].get(),
					RenderData.MaterialLibrary.Get("Terrain"),
					meshTag+"_Args",
					constCbvOffset,
					transform
				);

				RenderData.OpaqueRenderItems.push_back(RenderData.Terrain.get());
				//RenderData.RenderItems.push_back(std::move(RenderData.Terrain));
			}
		}
	}

	void Renderer3D::DeleteVoxelTerrain(const std::string& tag)
	{}


	void Renderer3D::CreateMesh(const std::string& meshTag, Transform transform, INT8 staticMeshType)
	{

		switch(staticMeshType)
		{
		case 0:
			{
				CreateCube(2, 2, 2, const_cast<std::string&>(meshTag));
			}
			break;
		case 1:
			{
				CreateSphere(0.4,const_cast<std::string&>(meshTag));
			}
			break;
			
		}

		ScopePointer<RenderItem> renderItem = RenderItem::Create
		(
			RenderData.Geometries[meshTag].get(),
			RenderData.MaterialLibrary.Get("Green"),
			meshTag+"_args",
			RenderData.OpaqueRenderItems.size(),
			transform
		);

		RenderData.OpaqueRenderItems.push_back(renderItem.get());
		RenderData.RenderItems.push_back(std::move(renderItem));
	}

	void Renderer3D::SetBuffer(const std::string& renderItemTag, 
	                           const std::vector<Vertex>& vertices,
	                           const std::vector<UINT16>& indices
	)
	{
		for(INT32 i=0;i<RenderData.OpaqueRenderItems.size();++i)
		{
			if(RenderData.OpaqueRenderItems[i]->Geometry->GetName()==renderItemTag)
			{
				if(renderItemTag=="Voxel")
				{
					VoxelStats.TriCount = vertices.size() * 3;
					VoxelStats.VertexCount = vertices.size();
				}

				const auto ri = RenderData.OpaqueRenderItems[i];
				ri->NumFramesDirty++;
				const auto item = RenderData.Geometries.at(renderItemTag).get();
				item->VertexBuffer->SetData(vertices.data(), vertices.size() * sizeof(Vertex), vertices.size());
				item->IndexBuffer->SetData(indices.data(), vertices.size());
				break;
			}
		}
	}


	RenderItem* Renderer3D::GetRenderItem(UINT16 index)
	{
		return RenderData.OpaqueRenderItems[index];
	}

	void Renderer3D::CreateOctreeDebugFrame(const std::vector<Vertex>& nodePositions)
	{
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
