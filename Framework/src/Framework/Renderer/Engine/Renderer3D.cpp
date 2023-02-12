#include "Renderer3D.h"


#include "GeometryGenerator.h"
#include "RenderInstruction.h"


#include <DirectXColors.h>

#include "Framework/Renderer/Api/FrameResource.h"
#include "Framework/Renderer/Buffers/VertexArray.h"
#include "Framework/Renderer/Pipeline/PipelineStateObject.h"
#include "Framework/Renderer/Resources/RenderItems.h"
#include "Framework/Renderer/Resources/Shader.h"
#include "Framework/Renderer/Resources/Material.h"
#include "IsoSurface/PerlinCompute.h"

#include "IsoSurface/VoxelWorld.h"

namespace Engine
{

	Renderer3D::RenderingStats Renderer3D::ProfileStats;
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


		ScopePointer<ResourceBuffer> UploadBufferManager;


		std::unordered_map<std::string, RefPointer<PipelineStateObject>> PSOs;
		RefPointer<PipelineStateObject> Pso;

		// Struct capturing performance
		Renderer3D::RenderingStats RendererStats;

		class VoxelWorld* voxelWorld;
		PerlinCompute* PerlinCompute;
	};

	static RendererData RenderData;


	void Renderer3D::PreInit()
	{
		/** get the graphics context */
		const auto api = RenderInstruction::GetApiPtr();

		/** for apis such as DirectX12 and Vulkan, we need reset the command list before submitting instructions */
		api->ResetCommandList();
	}

	void Renderer3D::PostInit()
	{
		/** get the graphics context */
		const auto api = RenderInstruction::GetApiPtr();

		/** tell the GPU to execute the init commands and wait until we're finished */
		api->ExecCommandList();
	}

	void Renderer3D::Init()
	{
		/** build and compile shaders */
		RenderData.VertexShader  = Shader::Create(L"assets\\shaders\\Default.hlsl",  "VS", "vs_5_1");
		RenderData.PixelShader   = Shader::Create(L"assets\\shaders\\Default.hlsl",  "PS", "ps_5_1");

		RenderData.ShaderLibrary.Add("vs", std::move(RenderData.VertexShader));
		RenderData.ShaderLibrary.Add("ps", std::move(RenderData.PixelShader));

		RenderData.VertexShader.reset();
		RenderData.PixelShader.reset();

		/**  Build the scene geometry  */
		const auto api = RenderInstruction::GetApiPtr();

		RenderData.voxelWorld = new class VoxelWorld();
		RenderData.voxelWorld->ComputeShader = Shader::Create(L"assets\\shaders\\MarchingCube.hlsl", "GenerateChunk", "cs_5_0");
		RenderData.voxelWorld->Init(api->GetGraphicsContext(), api->GetMemoryManager());

		RenderData.PerlinCompute = new PerlinCompute();
		RenderData.PerlinCompute->PerlinShader = Shader::Create(L"assets\\shaders\\Perlin.hlsl", "ComputeNoise3D", "cs_5_0");
		RenderData.PerlinCompute->Init(api->GetGraphicsContext(), api->GetMemoryManager());

		/*Initialise a plane*/
		
		

		/** create the pso */
		const BufferLayout layout =
		{
			{"POSITION",	ShaderDataType::Float3, 0,  0},
			{"NORMAL",		ShaderDataType::Float3, 12, 1},
			{"TEXCOORD",	ShaderDataType::Float2, 24, 2},
		};

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

	}

	void Renderer3D::Shutdown()
	{}

	void Renderer3D::BeginScene(const MainCamera& cam, const float deltaTime, const float elapsedTime)
	{

		static bool init = false;
		static bool build = false;
		if (!init)
		{
			if(!build)
				BuildMaterials();


			PerlinArgs pa;
			pa.Gain = 2.f;
			pa.Loss = 0.5f;
			pa.Octaves = 8;
			RenderData.PerlinCompute->Generate3DTexture(pa, VoxelWorldSize, VoxelWorldSize, VoxelWorldSize);


			/*generate one chunk*/
			const auto data = RenderData.voxelWorld->GenerateChunk({ 0.0f,0.f,0.f }, RenderData.PerlinCompute->ScalarTexture.get());

			if(!build)
			{
				RenderInstruction::ResetGraphicsCommandList();

				build = true;
				const auto api = RenderInstruction::GetApiPtr();
				BuildVoxelWorld(data, api->GetGraphicsContext());

				init = true;

				BuildRenderItems(api->GetGraphicsContext());
				BuildFrameResources(api->GetGraphicsContext());

				RenderData.UploadBufferManager = ResourceBuffer::Create
				(
					api->GetGraphicsContext(),
					RenderData.FrameResources,
					RenderData.RenderItems.size()
				);

				RenderInstruction::ExecGraphicsCommandList();
			}
		}

		if(!RenderData.FrameResources.empty())
		{
			RenderData.CurrentFrameResourceIndex = (RenderData.CurrentFrameResourceIndex + 1) % NUMBER_OF_FRAME_RESOURCES;
			RenderData.ActiveFrameResource = RenderData.FrameResources[RenderData.CurrentFrameResourceIndex].get();

			RenderInstruction::UpdateFrameResource(RenderData.ActiveFrameResource);
			RenderData.UploadBufferManager->UpdateCurrentFrameResource(RenderData.ActiveFrameResource);
			RenderData.UploadBufferManager->UpdateObjectBuffers(RenderData.OpaqueRenderItems);
			RenderData.UploadBufferManager->UpdateMaterialBuffers(RenderData.Materials);
			RenderData.UploadBufferManager->UpdatePassBuffer(cam, deltaTime, elapsedTime);

			RenderInstruction::PreRender();
			RenderInstruction::SetClearColour(NULL, RenderData.PSOs["Opaque"].get());
			RenderInstruction::DrawOpaqueItems(RenderData.OpaqueRenderItems, RenderData.CurrentFrameResourceIndex);
		}
	}

	void Renderer3D::EndScene()
	{
		/**
		 *	Render the scene geometry to the scene
		 */
		if(!RenderData.FrameResources.empty())
		{
			ProfileStats.DrawCalls = 1;
			RenderInstruction::Flush();
			RenderInstruction::PostRender();
		}
	}

	void Renderer3D::BuildRenderItems(GraphicsContext* graphicsContext)
	{
		float x = -10.f, y = -10.f, z = -10.f;
		UINT baseVertexLocation = 0;
		for (auto& geo : RenderData.Geometries)
		{
			auto renderItem = RenderItem::Create
			(
				geo.second.get(),
				RenderData.MaterialLibrary.Get("Default"),
				geo.first.c_str(),
				1,
				Transform(x, y, z)
			);

			renderItem->IndexCount = renderItem->Geometry->IndexBuffer->GetCount();
			renderItem->BaseVertexLocation = baseVertexLocation;
			baseVertexLocation += renderItem->Geometry->VertexBuffer->GetCount();

			RenderData.RenderItems.push_back(std::move(renderItem));
		}

	

		for (auto& renderItem : RenderData.RenderItems)
		{
			RenderData.OpaqueRenderItems.push_back(renderItem.get());
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

	void Renderer3D::BuildVoxelWorld(const std::vector<MCTriangle> triangles, GraphicsContext* gContext)
	{
		if (RenderData.Geometries.find("Terrain") == RenderData.Geometries.end())
		{
			std::vector<MCVertex> vertices;
			std::vector<UINT16> indices;
			UINT16 index = 0;
			for(auto& tri : triangles)
			{
				vertices.push_back(tri.VertexC);
				indices.push_back(++index);
				vertices.push_back(tri.VertexB);
				indices.push_back(++index);
				vertices.push_back(tri.VertexA);
				indices.push_back(++index);
			}

			const UINT ibSizeInBytes = sizeof(UINT16) * indices.size();
			const UINT vbSizeInBytes = sizeof(MCVertex) * vertices.size();

			ScopePointer<MeshGeometry> Terrain = CreateScope<MeshGeometry>("Terrain");
			Terrain->VertexBuffer = VertexBuffer::Create(gContext, vertices.data(),
				vbSizeInBytes, vertices.size(), true);

			const BufferLayout layout =
			{
				{"POSITION",	ShaderDataType::Float3, 0, 0, false },
				{"NORMAL",		ShaderDataType::Float3, 12,1, false },
				{"TEXCOORD",	ShaderDataType::Float2, 24,2, false },
			};
			Terrain->VertexBuffer->SetLayout(layout);

			Terrain->IndexBuffer = IndexBuffer::Create(gContext, indices.data(),
				ibSizeInBytes, indices.size());
		
			RenderData.Geometries.emplace("Terrain", std::move(Terrain));

			ScopePointer<RenderItem> mcGeo = RenderItem::Create
			(
				RenderData.Geometries["Terrain"].get(),
				RenderData.MaterialLibrary.Get("Default"),
				"Terrain",
				1,
				Transform(0, 0, 0)
			);

			mcGeo->IndexCount = indices.size();
			mcGeo->BaseVertexLocation = 0;
			mcGeo->StartIndexLocation = 0;

			RenderData.OpaqueRenderItems.push_back(mcGeo.get());
			RenderData.RenderItems.push_back(std::move(mcGeo));
		}
		else
		{

			auto terrain = RenderData.Geometries.at("Terrain").get();

			D3D12VertexBuffer* buffer = dynamic_cast<D3D12VertexBuffer*>(terrain->VertexBuffer.get());
			/*copy new vertices here!?*/

			//TODO: Maybe incorporate a chunks vertices into the update loop!
			//TODO: Like updating the materials and other geo.

		}
		

	}

	void Renderer3D::BuildFrameResources(GraphicsContext* graphicsContext)
	{
		for (int i = 0; i < NUMBER_OF_FRAME_RESOURCES; ++i)
		{
			RenderData.FrameResources.push_back(FrameResource::Create
			(
				graphicsContext,
				1,
				static_cast<UINT>(RenderData.RenderItems.size()),
				static_cast<UINT>(RenderData.MaterialLibrary.Size())
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
