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

#include "IsoSurface/VoxelWorld.h"

namespace Engine
{

	Renderer3D::RenderingStats Renderer3D::ProfileStats;

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

		ScopePointer<MeshGeometry> Box;

		std::unordered_map<std::string, RefPointer<PipelineStateObject>> PSOs;
		RefPointer<PipelineStateObject> Pso;

		// Struct capturing performance
		Renderer3D::RenderingStats RendererStats;

		class VoxelWorld* voxelWorld;
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


	
		BuildScalarField();
		PolygoniseScalarField();
		BuildMCBuffers();
	

		/*RenderData.voxelWorld = new class VoxelWorld();
		RenderData.voxelWorld->ComputeShader = Shader::Create(L"assets\\shaders\\MarchingCube.hlsl", "GenerateChunk", "cs_5_0");
		RenderData.voxelWorld->Init(api->GetGraphicsContext());*/

		///*
		// * The buffer size can be given by...
		// *
		// * '	(X - 1) * (Y - 1) * (Z - 1) * 5 * sizeof(Triangle)	'
		// *
		// */

		/**
		 * Build object materials.
		 */
		BuildMaterials();

		/**
		 * Build scene render items
		 */
		BuildRenderItems(api->GetGraphicsContext());

		/**
		 * Build scene frame resources
		 */
		BuildFrameResources(api->GetGraphicsContext());


		/**
		 * Build upload buffers
		 */
		RenderData.UploadBufferManager = ResourceBuffer::Create
		(
			api->GetGraphicsContext(),
			RenderData.FrameResources,
			RenderData.RenderItems.size()
		);

		/** create the pso */


		/** build the pipeline state objects */
		RenderData.PSOs.emplace("Opaque", PipelineStateObject::Create
		(
			api->GetGraphicsContext(),
			RenderData.ShaderLibrary.Get("vs"),
			RenderData.ShaderLibrary.Get("ps"),
			RenderData.Geometries["McGeo"]->VertexBuffer->GetLayout(),
			FillMode::Opaque
		));

	}

	void Renderer3D::Shutdown()
	{}

	void Renderer3D::BeginScene(const MainCamera& cam, const float deltaTime, const float elapsedTime)
	{
		

		/**
		 *	Cycle throught the frame resources array
		 */
		RenderData.CurrentFrameResourceIndex = (RenderData.CurrentFrameResourceIndex + 1) % NUMBER_OF_FRAME_RESOURCES;
		RenderData.ActiveFrameResource = RenderData.FrameResources[RenderData.CurrentFrameResourceIndex].get();

		/**
		 * Update the underlying fence and check if the GPU has finished executing
		 * commands for the current frame resource.
		 */
		RenderInstruction::UpdateFrameResource(RenderData.ActiveFrameResource);



		/**
		 * Update the interal frame resource pointer
		 */
		RenderData.UploadBufferManager->UpdateCurrentFrameResource(RenderData.ActiveFrameResource);

		/**
		 * Update each render item's CB if a change has been made
		 */
		RenderData.UploadBufferManager->UpdateObjectBuffers(RenderData.OpaqueRenderItems);

		/**
		 * Update each material in the scene
		 */
		RenderData.UploadBufferManager->UpdateMaterialBuffers(RenderData.Materials);

		/**
		 * Update the main pass buffer
		 */
		RenderData.UploadBufferManager->UpdatePassBuffer(cam, deltaTime, elapsedTime);


		RenderInstruction::PreRender();

		/*generate one chunk*/
		//RenderData.voxelWorld->GenerateChunk({ 0.0f,0.f,0.f });
		/*take the chunk data and copy it into the vertex buffer*/
		/**
		 * Clear the back buffer ready for rendering
		 */
		RenderInstruction::SetClearColour(NULL, RenderData.PSOs["Opaque"].get());

		/**
		 * Prepare scene geometry to the screen
		 */
		RenderInstruction::DrawOpaqueItems(RenderData.OpaqueRenderItems, RenderData.CurrentFrameResourceIndex);


	}

	void Renderer3D::EndScene()
	{
		/**
		 *	Render the scene geometry to the scene
		 */
		ProfileStats.DrawCalls = 1;
		RenderInstruction::Flush();
		RenderInstruction::PostRender();
	}

	void Renderer3D::BuildRenderItems(GraphicsContext* graphicsContext)
	{

		auto mcGeo = RenderItem::Create
		(
			RenderData.Geometries["McGeo"].get(),
			RenderData.MaterialLibrary.Get("Default"),
			"March",
			1,
			Transform(-10.0f, -10.0f, -10.0f)
		);


		RenderData.RenderItems.push_back(std::move(mcGeo));

		/**
		 * Store all the render items in an array
		 */
		for (auto& renderItem : RenderData.RenderItems)
		{
			RenderData.OpaqueRenderItems.push_back(renderItem.get());
		}
	}

	void Renderer3D::BuildMaterials()
	{

		auto matB = Material::Create("Default");
		matB->SetAlbedo(0.8f, 0.8f, 0.8f);
		matB->SetFresnel(0.02f, 0.02f, 0.02f);
		matB->SetRoughness(0.5f);
		matB->SetBufferIndex(1);
		RenderData.MaterialLibrary.Add("Default", std::move(matB));
		RenderData.Materials.push_back(RenderData.MaterialLibrary.Get("Default"));
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
				static_cast<UINT>(RenderData.MaterialLibrary.Size()),
				i
			)
			);
		}
	}





	const UINT32 Dimensions = 32;
	static constexpr  UINT32 WorldSize = Dimensions * Dimensions * Dimensions;

	constexpr float WorldResolution = 0.5f;

	////////////////////////////////////////////////////////
	

	static double IsoValue = 0;
	static std::vector<Voxel> VoxelWorld;
	static std::vector<Triangle> Triangles;

	void Renderer3D::BuildScalarField()
	{
		
		VoxelWorld.resize(WorldSize);

		const UINT32 DEPTH = Dimensions;
		const UINT32 WIDTH = Dimensions;
		const UINT32 BREADTH = Dimensions;

		/*
		 * For each voxel initialise the positions and values for each
		 * corner point.
		 */
		UINT32 count = 0;

		for (INT32 k = 0; k < DEPTH; ++k)
		{
			for (INT32 j = 0; j < WIDTH; ++j)
			{
				for (INT32 i = 0; i < BREADTH; ++i)
				{
					const UINT32 ijk = count;
					double freqGain = 2.0f;
					double amplitudeLoss = 0.5f;
					double freq = 0.03f;
					double amplitude = 100.0f;

					/**
					 * Simple FBM
					 */
					for (INT32 o = 0; o < 8; ++o)
					{
						/* A */
						VoxelWorld[ijk].Position[0] = XMFLOAT3(i, j, k);
						VoxelWorld[ijk].Value[0] += Perlin(i * freq, j * freq, k * freq) * amplitude;

						/* B */
						VoxelWorld[ijk].Position[1] = XMFLOAT3(i + 1, j, k);
						VoxelWorld[ijk].Value[1] += Perlin((i + 1) * freq, j * freq, k * freq) * amplitude;

						/* C */
						VoxelWorld[ijk].Position[2] = XMFLOAT3(i + 1, j + 1, k);
						VoxelWorld[ijk].Value[2] += Perlin((i + 1) * freq, (j + 1) * freq, k * freq) * amplitude;

						/* D */
						VoxelWorld[ijk].Position[3] = XMFLOAT3(i, j + 1, k);
						VoxelWorld[ijk].Value[3] += Perlin(i * freq, (j + 1) * freq, k * freq) * amplitude;


						/* E */
						VoxelWorld[ijk].Position[4] = XMFLOAT3(i, j, k - 1);
						VoxelWorld[ijk].Value[4] += Perlin(i * freq, j * freq, (k - 1) * freq) * amplitude;

						/* F */
						VoxelWorld[ijk].Position[5] = XMFLOAT3(i + 1, j, k - 1);
						VoxelWorld[ijk].Value[5] += Perlin((i + 1) * freq, j * freq, (k - 1) * freq) * amplitude;

						/* G */
						VoxelWorld[ijk].Position[6] = XMFLOAT3(i + 1, j + 1, k - 1);
						VoxelWorld[ijk].Value[6] += Perlin((i + 1) * freq, (j + 1) * freq, (k - 1) * freq) * amplitude;

						/* H */
						VoxelWorld[ijk].Position[7] = XMFLOAT3(i, j + 1, k - 1);
						VoxelWorld[ijk].Value[7] += Perlin(i * freq, (j + 1) * freq, (k - 1) * freq) * amplitude;

						freq *= freqGain;
						amplitude *= amplitudeLoss;

					}



					++count;
				}
			}
		}
	}

	void Renderer3D::PolygoniseScalarField()
	{

		MarchingCubesCPU mcSolver;

		const double isoLevelT = 3;

		for (const auto& voxel : VoxelWorld)
		{
			UINT32 count = mcSolver.PolygoniseScalarField(voxel, IsoValue, &Triangles);
		}
	}

	void Renderer3D::BuildMCBuffers()
	{
		const auto api = RenderInstruction::GetApiPtr();

		ScopePointer<MeshGeometry> mcGeo = CreateScope<MeshGeometry>("McGeo");

		std::vector<Vertex> vertices;
		std::vector<UINT16> indices;

		UINT index = 0;

		for(INT32 i = 0; i < Triangles.size(); ++i)
		{

			Vertex v;
			v.Position = Triangles[i].Vertex[0];
			v.Normal = Triangles[i].Normals[0];
			vertices.push_back(v);
			indices.push_back(index);

			index++;

			v.Position = Triangles[i].Vertex[1];
			v.Normal = Triangles[i].Normals[1];

			vertices.push_back(v);
			indices.push_back(index);

			index++;

			v.Position = Triangles[i].Vertex[2];
			v.Normal = Triangles[i].Normals[2];

			vertices.push_back(v);
			indices.push_back(index);

			ProfileStats.TriCount += 1;
			ProfileStats.PolyCount += 3;
			index++;
		}

		/*Build the index array*/


		/*Build CPU and GPU buffers*/
		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(UINT16);

		mcGeo->VertexBuffer = VertexBuffer::Create(api->GetGraphicsContext(), vertices.data(), vbByteSize, vertices.size(), true);
		mcGeo->VertexBuffer->SetLayout
		(
			{
				{"POSITION", ShaderDataType::Float3, 0 },
				{"NORMAL",	  ShaderDataType::Float3, 12 },
				{"TEXCOORD", ShaderDataType::Float2, 24 },
				
			}
		);

		
		mcGeo->IndexBuffer = IndexBuffer::Create(api->GetGraphicsContext(), indices.data(), ibByteSize, indices.size());

		SubGeometry sub;
		sub.BaseVertexLocation = 0;
		sub.IndexCount = mcGeo->IndexBuffer->GetCount();
		sub.StartIndexLocation = 0;

		mcGeo->DrawArgs["March"] = sub;

		/*Store the marching cubes algorithm into the global geometry handler*/
		RenderData.Geometries[mcGeo->GetName()] = std::move(mcGeo);

	}
}
