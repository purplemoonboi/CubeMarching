#include "Renderer3D.h"


#include "GeometryGenerator.h"
#include "RenderInstruction.h"

#include "Framework/Core/Compute/ComputeInstruction.h"
#include "Framework/Renderer/Api/FrameResource.h"
#include "Framework/Renderer/Buffers/VertexArray.h"
#include "Framework/Renderer/Pipeline/RenderPipeline.h"
#include "Framework/Renderer/Resources/RenderItems.h"
#include "Framework/Renderer/Resources/Shader.h"
#include "Framework/Renderer/Material/Material.h"
#include "Framework/Renderer/Textures/Texture.h"
#include "Framework/Scene/Scene.h"


namespace Foundation::Graphics
{

    GeometryGenerator Geo;

	struct RendererData
	{

		ScopePointer<VertexArray> VertexArray;

		ShaderLibrary ShaderLibrary;
		MaterialLibrary MaterialLibrary;
		TextureLibrary TextureLibrary;

		std::vector<Material*> Materials;
		std::vector<Texture*> Textures;

		std::unordered_map<std::string, ScopePointer<MeshGeometry>> Geometries;
		std::unordered_map<std::string, RenderPipeline> PSOs;


		std::vector<ScopePointer<RenderItem>> RenderItems;
		std::vector<RenderItem*> OpaqueRenderItems;
		std::vector<RenderItem*> WireFrameRenderItems;

		ScopePointer<RenderItem> Terrain;


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
		auto vS  = Shader::Create(L"assets\\shaders\\Default.hlsl", "VS", "vs_5_1");
		auto pS  = Shader::Create(L"assets\\shaders\\Default.hlsl", "PS", "ps_5_1");

		RenderData.ShaderLibrary.Add("vs", std::move(vS));
		RenderData.ShaderLibrary.Add("ps", std::move(pS));

		BuildCoreEngineDefaultTextures();
		BuildCoreEngineDefaultMaterials();

		PipelineDesc desc{};
		desc.Shaders[VERTEX_SHADER] = vS.get();
		desc.Shaders[PIXEL_SHADER] = pS.get();
		


		/** build the pipeline state objects */
		/*auto pipeline = RenderPipeline::Create
		(
			RenderData.ShaderLibrary.GetShader("vs"),
			RenderData.ShaderLibrary.GetShader("ps"),
			FillMode::Opaque
		);*/

		//RenderData.PSOs.emplace("Opaque", RenderPipeline::Create(desc));

		/*RenderPipeline::Create
		(
			RenderData.ShaderLibrary.GetShader("vs"),
			RenderData.ShaderLibrary.GetShader("ps"),
			FillMode::WireFrame
		);*/

		desc.Fill = FillMode::WireFrame;
		
		//RenderData.PSOs.emplace("Wire", RenderPipeline::Create(desc));

	}

	void Renderer3D::Shutdown()
	{}

	void Renderer3D::BeginScene(MainCamera* camera, AppTimeManager* time)
	{



		RenderInstruction::OnBeginRender();
	}

	void Renderer3D::EndScene()
	{
		/**
		 *	Render the scene geometry to the scene
		 */
		RenderInstruction::OnEndRender();
	}

	void Renderer3D::BuildCoreEngineDefaultMaterials()
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

	void Renderer3D::BuildCoreEngineDefaultTextures()
	{
		/**
		 *	moss
		 */
		auto mossAlbedo = Texture::Create(L"assets\\textures\\moss\\Moss_albedo.dds", "MossAlbedo");
		RenderData.Textures.push_back(mossAlbedo.get());
		RenderData.TextureLibrary.Add("MossAlbedo", std::move(mossAlbedo));

		auto mossNormal = Texture::Create(L"assets\\textures\\moss\\Moss_normal.dds", "MossNormal");
		RenderData.Textures.push_back(mossNormal.get());
		RenderData.TextureLibrary.Add("MossNormal", std::move(mossNormal));

		///**
		// *  rock
		// */
		auto rockAlbedo = Texture::Create(L"assets\\textures\\rock\\RocksLayered02_albedo.dds", "RockAlbedo");
		RenderData.Textures.push_back(rockAlbedo.get());
		RenderData.TextureLibrary.Add("RockAlbedo", std::move(rockAlbedo));

		auto rockNormal = Texture::Create(L"assets\\textures\\rock\\RocksLayered02_normal.dds", "RockNormal");
		RenderData.Textures.push_back(rockNormal.get());
		RenderData.TextureLibrary.Add("RockNormal", std::move(rockNormal));
	}

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


	RenderItem* Renderer3D::GetRenderItem(UINT16 index)
	{
		return RenderData.OpaqueRenderItems[index];
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
