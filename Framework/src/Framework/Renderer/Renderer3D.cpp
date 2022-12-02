#include "Renderer3D.h"

#include "Buffer.h"
#include "GeometryGenerator.h"
#include "RenderInstruction.h"
#include "VertexArray.h"
#include "Shader.h"
#include "RenderItems.h"
#include "PipelineStateObject.h"
#include "FrameResource.h"

#include <DirectXColors.h>

namespace Engine
{


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
		ScopePointer<Shader> ComputeShader;


		ShaderLibrary ShaderLibrary;

		std::unordered_map<std::string, ScopePointer<MeshGeometry>> Geometries;

		FrameResource* ActiveFrameResource;
		UINT32 CurrentFrameResourceIndex = 0;


		std::vector<ScopePointer<FrameResource>> FrameResources;
		std::vector<ScopePointer<RenderItem>> RenderItems;

		std::vector<RenderItem*> OpaqueRenderItems;


		ScopePointer<UploadBufferManager> UploadBufferManager;
		RefPointer<PipelineStateObject> Pso;

		ScopePointer<MeshGeometry> Box;

		std::unordered_map<std::string, RefPointer<PipelineStateObject>> PSOs;



		// Struct capturing performance
		Renderer3D::RenderingStats RendererStats;
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
		RenderData.VertexShader  = Shader::Create(L"assets\\shaders\\color.hlsl",  "VS", "vs_5_1");
		RenderData.PixelShader   = Shader::Create(L"assets\\shaders\\color.hlsl",  "PS", "ps_5_1");
		RenderData.ComputeShader = Shader::Create(L"assets\\shaders\\VecAdd.hlsl", "CS", "cs_5_0");

		RenderData.ShaderLibrary.Add("vs", std::move(RenderData.VertexShader));
		RenderData.ShaderLibrary.Add("ps", std::move(RenderData.PixelShader));
		RenderData.ShaderLibrary.Add("cs", std::move(RenderData.ComputeShader));

		RenderData.VertexShader.reset();
		RenderData.PixelShader.reset();
		RenderData.ComputeShader.reset();

		/**  Build the scene geometry  */

		GeometryGenerator geoGen;
		GeometryGenerator::MeshData box = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 0);

		// Define the SubmeshGeometry that cover different 
		// regions of the vertex/index buffers.

		SubGeometry boxSubmesh;
		boxSubmesh.IndexCount = (UINT)box.Indices32.size();
		boxSubmesh.StartIndexLocation = 0U;
		boxSubmesh.BaseVertexLocation = 0U;

		//
		// Extract the vertex elements we are interested in and pack the
		// vertices of all the meshes into one vertex buffer.
		//

		auto totalVertexCount = box.Vertices.size();

		std::vector<Vertex> vertices(totalVertexCount);

		UINT k = 0;
		for (UINT32 i = 0; i < box.Vertices.size(); ++i, ++k)
		{
			vertices[k].Position = box.Vertices[i].Position;
			vertices[k].Color = DirectX::XMFLOAT4(DirectX::Colors::Red);
		}

		const UINT16 totalIndexCount = static_cast<UINT16>(box.GetIndices16().size());
		std::vector<UINT16> indices(totalIndexCount);

		for (UINT32 i = 0; i < totalIndexCount; ++i)
		{
			indices[i] = box.Indices32[i];
		}



		ScopePointer<MeshGeometry> geo = MeshGeometry::Create("SceneGeo");

		/**
		 * @brief Create one large vertex and index buffer to store all the geometry.
		 */
		const auto api = RenderInstruction::GetApiPtr();
		
		const UINT vbByteSize = static_cast<UINT>(vertices.size()) * sizeof(Vertex);

		geo->VertexBuffer = VertexBuffer::Create(api->GetGraphicsContext(), vertices.data(), vbByteSize, static_cast<UINT>(vertices.size()));

		geo->VertexBuffer->SetLayout
		(
			{
			 {  "POSITION" , ShaderDataType::Float3	,	0	},
			 {  "COLOR"	    , ShaderDataType::Float4,  12	}
			}
		);

		const UINT ibByteSize = static_cast<UINT>(indices.size()) * sizeof(UINT16);

		geo->IndexBuffer = IndexBuffer::Create(api->GetGraphicsContext(), indices.data(), ibByteSize, static_cast<UINT>(indices.size()));

		/** store the attributes of each submesh */
		geo->DrawArgs["Box"] = boxSubmesh;


		RenderData.Geometries[geo->GetName()] = std::move(geo);


		BuildScalarField();
		PolygoniseScalarField();
		BuildMCBuffers();

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
		RenderData.UploadBufferManager = UploadBufferManager::Create
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
			RenderData.Geometries["SceneGeo"]->VertexBuffer->GetLayout(),
			FillMode::Opaque
		));

	}

	void Renderer3D::Shutdown()
	{}

	void Renderer3D::BeginScene(const MainCamera& cam, const DeltaTime& appTimeManager)
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
		 * Update the main pass buffer
		 */
		RenderData.UploadBufferManager->UpdatePassBuffer(cam, appTimeManager);







		/**
		 * Clear the back buffer ready for rendering
		 */
		RenderInstruction::SetClearColour(0, RenderData.PSOs["Opaque"].get());


		/**
		 * Prepare scene geometry to the screen
		 */
		RenderInstruction::DrawOpaqueItems(RenderData.OpaqueRenderItems, RenderData.CurrentFrameResourceIndex);


		//RenderInstruction::DrawIndexed(RenderData.Box, 0);
	}

	void Renderer3D::EndScene()
	{
		/**
		 *	Render the scene geometry to the scene
		 */
		RenderInstruction::Flush();
	}

	void Renderer3D::BuildRenderItems(GraphicsContext* graphicsContext)
	{

		auto boxRitem = RenderItem::Create
		(
			RenderData.Geometries["SceneGeo"].get(),
			"Box",
			0,
			Transform(0.0f, 0.0f, 0.0f)
		);

		RenderData.RenderItems.push_back(std::move(boxRitem));


		auto mcGeo = RenderItem::Create
		(
			RenderData.Geometries["mcGeo"].get(),
			"mcDA",
			1,
			Transform(0.0f, 0.0f, 0.0f)
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

	void Renderer3D::BuildFrameResources(GraphicsContext* graphicsContext)
	{
		for (int i = 0; i < NUMBER_OF_FRAME_RESOURCES; ++i)
		{
			RenderData.FrameResources.push_back(FrameResource::Create
			(
				graphicsContext,
				NUMBER_OF_FRAME_RESOURCES,
				static_cast<UINT>(RenderData.RenderItems.size()),
				i
			)
			);
		}
	}

	const UINT32 Dimensions = 32;
	static constexpr  UINT32 WorldSize = Dimensions * Dimensions * Dimensions;

	constexpr float WorldResolution = 0.1f;

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

		MarchingCubesSolver mcSolver;

		const double isoLevelT = 3;

		for (const auto& voxel : VoxelWorld)
		{
			UINT32 count = mcSolver.PolygoniseScalarField(voxel, isoLevelT, &Triangles);
		}
	}

	void Renderer3D::BuildMCBuffers()
	{
		const auto api = RenderInstruction::GetApiPtr();

		ScopePointer<MeshGeometry> mcGeo = CreateScope<MeshGeometry>("mcGeo");

		std::vector<Vertex> vertices;
		std::vector<UINT16> indices;

		UINT index = 0;

		for(INT32 i = 0; i < Triangles.size(); ++i)
		{

			Vertex v;
			v.Position = Triangles[i].Vertex[0];
			vertices.push_back(v);
			indices.push_back(index);

			index++;

			v.Position = Triangles[i].Vertex[1];
			vertices.push_back(v);
			indices.push_back(index);

			index++;

			v.Position = Triangles[i].Vertex[2];
			vertices.push_back(v);
			indices.push_back(index);


			index++;
		}

		/*Build the index array*/


		/*Build CPU and GPU buffers*/
		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(UINT16);

		mcGeo->VertexBuffer = VertexBuffer::Create(api->GetGraphicsContext(), vertices.data(), vbByteSize, vertices.size());
		mcGeo->VertexBuffer->SetLayout
		(
			{
				{"POSITION", ShaderDataType::Float3 },
				{"COLOR", ShaderDataType::Float4	}
			}
		);

		
		mcGeo->IndexBuffer = IndexBuffer::Create(api->GetGraphicsContext(), indices.data(), ibByteSize, indices.size());

		SubGeometry sub;
		sub.BaseVertexLocation = 0;
		sub.IndexCount = mcGeo->IndexBuffer->GetCount();
		sub.StartIndexLocation = 0;

		mcGeo->DrawArgs["mcDA"] = sub;

		/*Store the marching cubes algorithm into the global geometry handler*/
		RenderData.Geometries[mcGeo->GetName()] = std::move(mcGeo);

	}
}
