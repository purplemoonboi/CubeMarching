#include "GeometryEngine.h"


#include "GeometryGenerator.h"
#include "Framework/Renderer/Engine/RenderInstruction.h"

#include "Framework/Core/Compute/ComputeInstruction.h"
#include "Framework/Renderer/Api/FrameResource.h"
#include "Framework/Renderer/Buffers/VertexArray.h"
#include "Framework/Renderer/Pipeline/RenderPipeline.h"
#include "Framework/Renderer/Shaders/Shader.h"
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

		//GeometryEngine::RenderingStats RendererStats;

	};

	static RendererData RenderData;


	void GeometryEngine::PreInit()
	{
		RenderInstruction::PreInitRenderer();
	}

	void GeometryEngine::PostInit()
	{
		RenderInstruction::PostInitRenderer();
	}

	void GeometryEngine::Init()
	{
	}

	void GeometryEngine::Shutdown()
	{}

	void GeometryEngine::BeginScene(MainCamera* camera, AppTimeManager* time)
	{



	}

	void GeometryEngine::EndScene()
	{

	}

	void GeometryEngine::BuildCoreEngineDefaultMaterials()
	{

	}

	void GeometryEngine::BuildCoreEngineDefaultTextures()
	{

	}


	RenderItem* GeometryEngine::GetRenderItem(UINT16 index)
	{
		return RenderData.OpaqueRenderItems[index];
	}

	void GeometryEngine::CreateCube(float x, float y, float z, std::string& name, UINT32 subDivisions)
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

	void GeometryEngine::CreatePlane(float x, float y, float z, float w, float h, float depth, std::string& name, UINT32 subSivisions)
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

	void GeometryEngine::CreateSphere(float radius, std::string& name, UINT32 lateralResolution, UINT32 longitudeResolution)
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
