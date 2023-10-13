#include "Renderer3D.h"
#include "Framework/Core/core.h"

#include "GeometryGenerator.h"
#include "RenderInstruction.h"


#include <DirectXColors.h>

#include "Framework/Renderer/Api/FrameResource.h"
#include "Framework/Renderer/Buffers/VertexArray.h"
#include "Framework/Renderer/Shaders/Shader.h"
#include "Framework/Renderer/Material/Material.h"
#include "Framework/Renderer/Textures/Texture.h"
#include "Framework/Renderer/Pipeline/RenderPipeline.h"

namespace Foundation::Graphics
{

	GeometryGenerator Geo;

	struct RendererData
	{
		// Constant data
		static constexpr UINT32 MaxTriangles = 100000;
		static constexpr UINT32 MaxVertices = MaxTriangles * 4;
		static constexpr UINT32 MaxIndices = MaxTriangles * 6;
		static constexpr UINT32 MaxTextureSlots = 32;


		ShaderLibrary ShaderLibrary;
		MaterialLibrary MaterialLibrary;
		TextureLibrary TextureLibrary;

		std::vector<Material*> Materials;
		std::vector<Texture*> Textures;

		std::unordered_map<std::string, RefPointer<RenderPipeline>> PSOs;

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
	}

	void Renderer3D::Shutdown()
	{}

	void Renderer3D::BeginScene(const MainCamera& cam, const float deltaTime, bool wireframe, const float elapsedTime)
	{

	}

	void Renderer3D::EndScene()
	{
		/**
		 *	Render the scene geometry to the scene
		 */
		RenderInstruction::Flush();
	}

	void Renderer3D::BuildMaterials()
	{

	}

	void Renderer3D::CreateCube(float x, float y, float z, std::string& name, UINT32 subDivisions)
	{
		/*auto api = RenderInstruction::GetApiPtr();
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
		}*/
	}

	void Renderer3D::CreatePlane(float x, float y, float z, float w, float h, float depth, std::string& name, UINT32 subSivisions)
	{
		/*auto api = RenderInstruction::GetApiPtr();
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
		}*/
	}

	void Renderer3D::CreateSphere(float radius, std::string& name, UINT32 lateralResolution, UINT32 longitudeResolution)
	{
		/*	auto api = RenderInstruction::GetApiPtr();
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
			}*/
	}

}
