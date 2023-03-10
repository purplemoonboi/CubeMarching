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
#include "IsoSurface/PerlinCompute.h"

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
		std::vector<Material*> Materials;

		std::unordered_map<std::string, ScopePointer<MeshGeometry>> Geometries;

		std::vector<ScopePointer<FrameResource>> FrameResources;
		FrameResource* ActiveFrameResource;
		UINT32 CurrentFrameResourceIndex = 0;

		std::vector<ScopePointer<RenderItem>> RenderItems;
		std::vector<RenderItem*> OpaqueRenderItems;
		std::vector<RenderItem*> WireFrameRenderItems;


		ScopePointer<ResourceBuffer> UploadBufferManager;


		std::unordered_map<std::string, RefPointer<PipelineStateObject>> PSOs;

		UINT64 BaseVertexLocation = 0;

		//Renderer3D::RenderingStats RendererStats;

	};

	static RendererData RenderData;


	void Renderer3D::PreInit()
	{
		const auto api = RenderInstruction::GetApiPtr();
		api->ResetCommandList();
	}

	void Renderer3D::PostInit()
	{
		const auto api = RenderInstruction::GetApiPtr();
		api->ExecCommandList();
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

		BuildMaterials();


		/** build the pipeline state objects */
		RenderData.PSOs.emplace("Opaque", PipelineStateObject::Create
		(
			api->GetGraphicsContext(),
			RenderData.ShaderLibrary.Get("vs"),
			RenderData.ShaderLibrary.Get("ps"),
			layout,
			FillMode::Opaque
		));

		RenderData.PSOs.emplace("Wire", PipelineStateObject::Create
		(
			api->GetGraphicsContext(),
			RenderData.ShaderLibrary.Get("vs"),
			RenderData.ShaderLibrary.Get("ps"),
			layout,
			FillMode::WireFrame
		));

		auto gridData = Geo.CreateGrid(100, 100, 12, 12);
		auto gridMesh = MeshGeometry::Create("EditorGrid");
		gridMesh->VertexBuffer = VertexBuffer::Create(api->GetGraphicsContext(),
			gridData.Vertices.data(),
			sizeof(Vertex) * gridData.Vertices.size(),
			gridData.Vertices.size(),
			false);

		gridMesh->IndexBuffer = IndexBuffer::Create(api->GetGraphicsContext(),
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
		boxMesh->VertexBuffer = VertexBuffer::Create(api->GetGraphicsContext(),
			boxData.Vertices.data(),
			sizeof(Vertex) * boxData.Vertices.size(),
			boxData.Vertices.size(),
			false);

		boxMesh->IndexBuffer = IndexBuffer::Create(api->GetGraphicsContext(),
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


		RenderData.UploadBufferManager = ResourceBuffer::Create
		(
			api->GetGraphicsContext(),
			RenderData.FrameResources,
			RenderData.RenderItems.size()
		);


		BuildFrameResources(api->GetGraphicsContext());
	}

	void Renderer3D::Shutdown()
	{}

	void Renderer3D::BeginScene(const MainCamera& cam, const float deltaTime, const float elapsedTime)
	{
		if(!RenderData.FrameResources.empty())
		{
			RenderData.CurrentFrameResourceIndex = (RenderData.CurrentFrameResourceIndex + 1) % NUMBER_OF_FRAME_RESOURCES;
			RenderData.ActiveFrameResource = RenderData.FrameResources[RenderData.CurrentFrameResourceIndex].get();

			RenderInstruction::UpdateFrameResource(RenderData.ActiveFrameResource);
			RenderData.UploadBufferManager->UpdateCurrentFrameResource(RenderData.ActiveFrameResource);
			RenderData.UploadBufferManager->UpdateObjectBuffers(RenderData.OpaqueRenderItems);
			RenderData.UploadBufferManager->UpdateMaterialBuffers(RenderData.Materials);
			RenderData.UploadBufferManager->UpdatePassBuffer(cam, deltaTime, elapsedTime);
		}

		RenderInstruction::PreRender();

		RenderInstruction::BindGeometryPass(RenderData.PSOs["Opaque"].get(), RenderData.OpaqueRenderItems);

	}

	void Renderer3D::EndScene()
	{
		/**
		 *	Render the scene geometry to the scene
		 */
		if(!RenderData.FrameResources.empty())
		{
			//ProfileStats.DrawCalls = 1;
			RenderInstruction::Flush();
			RenderInstruction::PostRender();
		}
	}

	void Renderer3D::BuildMaterials()
	{
		auto mat = Material::Create("Green");
		mat->SetAlbedo(0.0f, 1.0f, 0.3f);
		mat->SetFresnel(0.0f, 0.0f, 1.0f);
		mat->SetRoughness(0.5f);
		mat->SetBufferIndex(0);
		RenderData.MaterialLibrary.Add("Green", std::move(mat));
		RenderData.Materials.push_back(RenderData.MaterialLibrary.Get("Green"));

		auto matB = Material::Create("Default");
		matB->SetAlbedo(0.8f, 0.8f, 0.8f);
		matB->SetFresnel(0.02f, 0.02f, 0.02f);
		matB->SetRoughness(0.5f);
		matB->SetBufferIndex(1);
		RenderData.MaterialLibrary.Add("Default", std::move(matB));
		RenderData.Materials.push_back(RenderData.MaterialLibrary.Get("Default"));
	}


	void Renderer3D::CreateCustomMesh
	(
		ScopePointer<MeshGeometry> meshGeometry, 
		const std::string& meshTag,
		Transform transform
	)
	{
	
		if (RenderData.Geometries.find(meshTag) != RenderData.Geometries.end())
		{
			RenderData.Geometries.erase(meshTag);
		}

		RenderData.Geometries.emplace(meshTag, std::move(meshGeometry));

		ScopePointer<RenderItem> customGeometry = RenderItem::Create
		(
			RenderData.Geometries[meshTag].get(),
			RenderData.MaterialLibrary.Get("Default"),
			meshTag,
			2,
			transform
		);

		customGeometry->IndexCount = RenderData.Geometries[meshTag]->IndexBuffer->GetCount();
		customGeometry->BaseVertexLocation = RenderData.BaseVertexLocation;
		//RenderData.BaseVertexLocation = customGeometry->Geometry->VertexBuffer->GetCount();

		UINT32 vertexOffset = 0;
		for (auto itr = RenderData.Geometries.begin(); itr != RenderData.Geometries.end(); itr++)
		{
			vertexOffset = itr->second->VertexBuffer->GetCount();
		}

		RenderData.BaseVertexLocation = vertexOffset;

		VoxelStats.PolyCount = customGeometry->Geometry->VertexBuffer->GetCount();
		VoxelStats.TriCount = VoxelStats.PolyCount / 3;

		const auto geoCount = RenderData.OpaqueRenderItems.size() - 1;
		RenderData.OpaqueRenderItems.at(geoCount)	= customGeometry.get();
		RenderData.RenderItems.at(geoCount)			= std::move(customGeometry);

		
	}

	void Renderer3D::BuildFrameResources(GraphicsContext* graphicsContext)
	{
		constexpr UINT32 maxObjCount = 16;
		constexpr UINT32 maxMatCount = 16;

		for (int i = 0; i < NUMBER_OF_FRAME_RESOURCES; ++i)
		{
			RenderData.FrameResources.push_back(FrameResource::Create
			(
				graphicsContext,
				1,
				maxMatCount,
				maxObjCount
			)
			);
		}
	}

	void Renderer3D::CreateCube(float x, float y, float z, std::string& name, UINT32 subDivisions)
	{
		auto api = RenderInstruction::GetApiPtr();
		auto cubeData = Geo.CreateBox(x, y, z, subDivisions);

		ScopePointer<MeshGeometry> cubeGeometry = CreateScope<MeshGeometry>(name);
		const UINT vbSizeInBytes = sizeof(Vertex) * cubeData.Vertices.size();
		const UINT ibSizeInBytes = sizeof(UINT16) * cubeData.GetIndices16().size();

		cubeGeometry->VertexBuffer = VertexBuffer::Create(api->GetGraphicsContext(), 
			cubeData.Vertices.data(), vbSizeInBytes, cubeData.Vertices.size(), false);

		BufferLayout layout = BufferLayout
		({ 
			{ "VERTEX",	  ShaderDataType::Float3, 0,   0 },
			{ "NORMAL",	  ShaderDataType::Float3, 12,  1 },
			{ "TEXCOORDS", ShaderDataType::Float2, 24, 2 } 
		});
		
		cubeGeometry->VertexBuffer->SetLayout(layout);

		cubeGeometry->IndexBuffer = IndexBuffer::Create(api->GetGraphicsContext(),
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

		planeGeometry->VertexBuffer = VertexBuffer::Create(api->GetGraphicsContext(),
			planeData.Vertices.data(), vbSizeInBytes, planeData.Vertices.size(), false);

		planeGeometry->IndexBuffer = IndexBuffer::Create(api->GetGraphicsContext(),
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

		sphereGeometry->VertexBuffer = VertexBuffer::Create(api->GetGraphicsContext(),
			sphereData.Vertices.data(), vbSizeInBytes, sphereData.Vertices.size(), false);

		sphereGeometry->IndexBuffer = IndexBuffer::Create(api->GetGraphicsContext(),
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
