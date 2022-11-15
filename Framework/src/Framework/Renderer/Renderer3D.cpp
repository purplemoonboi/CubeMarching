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
		static const UINT32 MaxTriangles = 100000;
		static const UINT32 MaxVertices = MaxTriangles * 4;
		static const UINT32 MaxIndices = MaxTriangles * 6;
		static const UINT32 MaxTextureSlots = 32;
		static const UINT32 MaxNumberOfFrameResources = 3;

		RefPointer<VertexArray> VertexArray;
		RefPointer<Shader> VertexShader;
		RefPointer<Shader> PixelShader;


		ShaderLibrary ShaderLib;

		std::unordered_map<std::string, RefPointer<MeshGeometry>> Geometries;

		RefPointer<FrameResource> ActiveFrameResource;

		std::vector<RefPointer<FrameResource>> FrameResources;
		std::vector<RefPointer<RenderItem>> RenderItems;
		std::vector<RefPointer<RenderItem>> OpaqueRenderItems;


		RefPointer<UploadBufferManager> UploadBufferManager;
		RefPointer<PipelineStateObject> Pso;


		std::unordered_map<std::string, RefPointer<PipelineStateObject>> PSOs;

		UINT32 CurrentFrameResourceIndex = 0;


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
		RenderData.VertexShader = Shader::Create(L"assets\\shaders\\color.hlsl", "VS", "vs_5_0");
		RenderData.PixelShader = Shader::Create(L"assets\\shaders\\color.hlsl", "PS", "ps_5_0");

		RenderData.ShaderLib.Add("vertex_shader", RenderData.VertexShader);

		GeometryGenerator geoGen;
		GeometryGenerator::MeshData box = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 3);
		GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
		GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
		GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

		//
		// We are concatenating all the geometry into one big vertex/index buffer.  So
		// define the regions in the buffer each submesh covers.
		//

		// Cache the vertex offsets to each object in the concatenated vertex buffer.
		UINT boxVertexOffset = 0;
		UINT gridVertexOffset = (UINT)box.Vertices.size();
		UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
		UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();

		// Cache the starting index for each object in the concatenated index buffer.
		UINT boxIndexOffset = 0;
		UINT gridIndexOffset = (UINT)box.Indices32.size();
		UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
		UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();

		// Define the SubmeshGeometry that cover different 
		// regions of the vertex/index buffers.

		SubGeometry boxSubmesh;
		boxSubmesh.IndexCount = (UINT)box.Indices32.size();
		boxSubmesh.StartIndexLocation = boxIndexOffset;
		boxSubmesh.BaseVertexLocation = boxVertexOffset;

		SubGeometry gridSubmesh;
		gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
		gridSubmesh.StartIndexLocation = gridIndexOffset;
		gridSubmesh.BaseVertexLocation = gridVertexOffset;

		SubGeometry sphereSubmesh;
		sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
		sphereSubmesh.StartIndexLocation = sphereIndexOffset;
		sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

		SubGeometry cylinderSubmesh;
		cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
		cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
		cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

		//
		// Extract the vertex elements we are interested in and pack the
		// vertices of all the meshes into one vertex buffer.
		//

		auto totalVertexCount =
			box.Vertices.size() +
			grid.Vertices.size() +
			sphere.Vertices.size() +
			cylinder.Vertices.size();

		std::vector<Vertex> vertices(totalVertexCount);

		UINT k = 0;
		for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
		{
			vertices[k].Position = box.Vertices[i].Position;
			vertices[k].Color = DirectX::XMFLOAT4(DirectX::Colors::DarkGreen);
		}

		for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
		{
			vertices[k].Position = grid.Vertices[i].Position;
			vertices[k].Color = DirectX::XMFLOAT4(DirectX::Colors::ForestGreen);
		}

		for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
		{
			vertices[k].Position = sphere.Vertices[i].Position;
			vertices[k].Color = DirectX::XMFLOAT4(DirectX::Colors::Crimson);
		}

		for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
		{
			vertices[k].Position = cylinder.Vertices[i].Position;
			vertices[k].Color = DirectX::XMFLOAT4(DirectX::Colors::SteelBlue);
		}

		std::vector<UINT16> indices;
		indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
		indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
		indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
		indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));

		const UINT vbByteSize = static_cast<UINT>(vertices.size()) * sizeof(Vertex);
		const UINT ibByteSize = static_cast<UINT>(indices.size()) * sizeof(UINT16);

		RefPointer<MeshGeometry> geo = MeshGeometry::Create("SceneGeo");

		/**
		 * @brief Create one large vertex and index buffer to store all the geometry.
		 */
		 /** get the graphics context */
		const auto api = RenderInstruction::GetApiPtr();

		RenderData.UploadBufferManager = UploadBufferManager::Create(api->GetGraphicsContext(), RenderData.FrameResources.data()->get(), 1, true, 3, RenderData.RenderItems.size());

		geo->VBuffer = VertexBuffer::Create(api->GetGraphicsContext(), vertices.data(), vbByteSize, static_cast<UINT>(vertices.size()));
		geo->VBuffer->SetLayout
		({
			 {  "POSITION" , ShaderDataType::Float3	,	0},
			 {  "COLOR"	    , ShaderDataType::Float4,  12	}
			}
		);

		geo->IBuffer = IndexBuffer::Create(api->GetGraphicsContext(), indices.data(), ibByteSize, static_cast<UINT>(indices.size()));

		/** store the attributes of each submesh */
		geo->DrawArgs["Box"] = boxSubmesh;
		geo->DrawArgs["Grid"] = gridSubmesh;
		geo->DrawArgs["Sphere"] = sphereSubmesh;
		geo->DrawArgs["Cylinder"] = cylinderSubmesh;

		RenderData.Geometries[geo->GetName()] = std::move(geo);

		/** create the pso */


		///** build the pipeline state objects */
		//RenderData.PSOs.emplace("Opaque", PipelineStateObject::Create
		//(
		//	api->GetGraphicsContext(),
		//	RenderData.VertexShader.get(),
		//	RenderData.PixelShader.get(),
		//	RenderData.Geometries["SceneGeo"]->VertexBuffer->GetLayout(),
		//	FillMode::Opaque
		//));

		//RenderData.PSOs.emplace("WireFrame", PipelineStateObject::Create
		//(
		//	api->GetGraphicsContext(),
		//	RenderData.VertexShader.get(),
		//	RenderData.PixelShader.get(),
		//	RenderData.Geometries["SceneGeo"]->VertexBuffer->GetLayout(),
		//	FillMode::WireFrame
		//));


	
	}

	void Renderer3D::Shutdown()
	{
	}


	void Renderer3D::BeginScene(const MainCamera& cam, const AppTimeManager& appTimeManager)
	{
		//Update the world matrices
		for(auto& constBuffer : RenderData.FrameResources)
		{
			
		}

		RenderData.UploadBufferManager->Update(cam, appTimeManager);


		RenderInstruction::SetClearColour(0, RenderData.PSOs["Opaque"].get());


		// Render the geometry and use the current Pso settings (Ignore the count (36) for now.)
		//RenderInstruction::DrawIndexed(RenderData.Geometries["Box"], 36);
		RenderInstruction::DrawRenderItems(RenderData.ActiveFrameResource.get(), RenderData.RenderItems);

	}

	void Renderer3D::EndScene()
	{
		RenderInstruction::Flush();
		
	}

	void Renderer3D::BuildRenderItems(GraphicsContext* graphicsContext)
	{

		auto boxRitem = RenderItem::Create
		(
			RenderData.Geometries["SceneGeo"], 
			"Box", 
			0, 
			Transform(0.0f, 0.5f, 0.0f, 0, 0, 0, 2.0f, 2.0f, 2.0f)
		);

		RenderData.RenderItems.push_back(std::move(boxRitem));

		auto gridRitem = RenderItem::Create
		(
			RenderData.Geometries["SceneGeo"],
			"Grid",
			1,
			Transform(0.0f, 0.0f, 0.0f)
		);

		RenderData.RenderItems.push_back(std::move(gridRitem));


		UINT objCBIndex = 2;

		for (int i = 0; i < 5; ++i)
		{
			auto leftCylRitem = RenderItem::Create
			(
				RenderData.Geometries["SceneGeo"],
				"Cylinder",
				objCBIndex,
				Transform(-5.0f, 1.5f, -10.0f + i * 5.0f)
			);


			auto rightCylRitem = RenderItem::Create
			(
				RenderData.Geometries["SceneGeo"],
				"Cylinder",
				objCBIndex++,
				Transform(+5.0f, 1.5f, -10.0f + i * 5.0f)
			);


			auto leftSphereRitem = RenderItem::Create
			(
				RenderData.Geometries["SceneGeo"],
				"Sphere",
				objCBIndex++,
				Transform(-5.0f, 3.5f, -10.0f + i * 5.0f)
			);

			auto rightSphereRitem = RenderItem::Create
			(
				RenderData.Geometries["SceneGeo"],
				"Sphere",
				objCBIndex++,
				Transform(+5.0f, 3.5f, -10.0f + i * 5.0f)
			);


			RenderData.RenderItems.push_back(std::move(leftCylRitem));
			RenderData.RenderItems.push_back(std::move(rightCylRitem));
			RenderData.RenderItems.push_back(std::move(leftSphereRitem));
			RenderData.RenderItems.push_back(std::move(rightSphereRitem));
		}

		// All the render items are opaque.
		for (auto& renderItem : RenderData.RenderItems)
		{
			RenderData.OpaqueRenderItems.push_back(renderItem);
		}
	}

	void Renderer3D::BuildFrameResources(GraphicsContext* graphicsContext)
	{
		for (int i = 0; i < RendererData::MaxNumberOfFrameResources; ++i)
		{
			RenderData.FrameResources.push_back
			(
				CreateScope<FrameResource>(graphicsContext, 1, static_cast<UINT>(RenderData.RenderItems.size()))
			);
		}
	}

}
