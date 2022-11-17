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

		FrameResource* ActiveFrameResource;
		UINT32 CurrentFrameResourceIndex = 0;


		std::vector<ScopePointer<FrameResource>> FrameResources;
		std::vector<ScopePointer<RenderItem>> RenderItems;
		std::vector<RenderItem*> OpaqueRenderItems;


		RefPointer<UploadBufferManager> UploadBufferManager;
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
		RenderData.VertexShader = Shader::Create(L"assets\\shaders\\color.hlsl", "VS", "vs_5_1");
		RenderData.PixelShader = Shader::Create(L"assets\\shaders\\color.hlsl", "PS", "ps_5_1");

	//	RenderData.ShaderLib.Add("vertex_shader", RenderData.VertexShader);

		/**  Build the scene geometry  */

		GeometryGenerator geoGen;
		GeometryGenerator::MeshData box = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 0);

		// Cache the vertex offsets to each object in the concatenated vertex buffer.
		UINT boxVertexOffset = 0;

		// Cache the starting index for each object in the concatenated index buffer.
		UINT boxIndexOffset = 0;


		// Define the SubmeshGeometry that cover different 
		// regions of the vertex/index buffers.

		SubGeometry boxSubmesh;
		boxSubmesh.IndexCount = (UINT)box.Indices32.size();
		boxSubmesh.StartIndexLocation = boxIndexOffset;
		boxSubmesh.BaseVertexLocation = boxVertexOffset;

		//
		// Extract the vertex elements we are interested in and pack the
		// vertices of all the meshes into one vertex buffer.
		//

		auto totalVertexCount = box.Vertices.size();

		std::vector<Vertex> vertices(totalVertexCount);

		UINT k = 0;
		for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
		{
			vertices[k].Position = box.Vertices[i].Position;
			vertices[k].Color = DirectX::XMFLOAT4(DirectX::Colors::DarkGreen);
		}

		std::vector<UINT16> indices;
		indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));


		RefPointer<MeshGeometry> geo = MeshGeometry::Create("SceneGeo");

		/**
		 * @brief Create one large vertex and index buffer to store all the geometry.
		 */
		const auto api = RenderInstruction::GetApiPtr();
		
		const UINT vbByteSize = static_cast<UINT>(vertices.size()) * sizeof(Vertex);

		geo->VBuffer = VertexBuffer::Create(api->GetGraphicsContext(), vertices.data(), vbByteSize, static_cast<UINT>(vertices.size()));

		geo->VBuffer->SetLayout
		(
			{
			 {  "POSITION" , ShaderDataType::Float3	,	0	},
			 {  "COLOR"	    , ShaderDataType::Float4,  12	}
			}
		);

		const UINT ibByteSize = static_cast<UINT>(indices.size()) * sizeof(UINT16);

		geo->IBuffer = IndexBuffer::Create(api->GetGraphicsContext(), indices.data(), ibByteSize, static_cast<UINT>(indices.size()));

		/** store the attributes of each submesh */
		geo->DrawArgs["Box"] = boxSubmesh;


		RenderData.Geometries[geo->GetName()] = std::move(geo);


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
			RenderData.FrameResources.data(),
			1,
			true,
			NUM_OF_RESOURCES,
			RenderData.RenderItems.size()
		);

		/** create the pso */


		/** build the pipeline state objects */
		RenderData.PSOs.emplace("Opaque", PipelineStateObject::Create
		(
			api->GetGraphicsContext(),
			RenderData.VertexShader.get(),
			RenderData.PixelShader.get(),
			RenderData.Geometries["SceneGeo"]->VBuffer->GetLayout(),
			FillMode::Opaque
		));

		RenderData.PSOs.emplace("WireFrame", PipelineStateObject::Create
		(
			api->GetGraphicsContext(),
			RenderData.VertexShader.get(),
			RenderData.PixelShader.get(),
			RenderData.Geometries["SceneGeo"]->VBuffer->GetLayout(),
			FillMode::WireFrame
		));

	
	}

	void Renderer3D::Shutdown()
	{}

	void Renderer3D::BeginScene(const MainCamera& cam, const DeltaTime& appTimeManager)
	{

		/**
		 *	Cycle throught the frame resources array
		 */
		RenderData.CurrentFrameResourceIndex = (RenderData.CurrentFrameResourceIndex + 1) % NUM_OF_RESOURCES;
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
		RenderData.UploadBufferManager->UpdateConstantBuffer(RenderData.OpaqueRenderItems);


		/**
		 * Update the main pass buffer
		 */
		RenderData.UploadBufferManager->Update(cam, appTimeManager);







		/**
		 * Clear the back buffer ready for rendering
		 */
		RenderInstruction::SetClearColour(0, RenderData.PSOs["Opaque"].get());


		/**
		 * Prepare scene geometry to the screen
		 */
		RenderInstruction::DrawRenderItems(RenderData.OpaqueRenderItems, RenderData.CurrentFrameResourceIndex, RenderData.OpaqueRenderItems.size());


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
			Transform(10.0f, 0.5f, 0.0f, 0, 0, 0, 2.0f, 2.0f, 2.0f)
		);

		RenderData.RenderItems.push_back(std::move(boxRitem));

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
		const char tags[3]{ 'A', 'B', 'C' };
		for (int i = 0; i < RendererData::MaxNumberOfFrameResources; ++i)
		{
			RenderData.FrameResources.push_back(FrameResource::Create(graphicsContext, 1, static_cast<UINT>(RenderData.RenderItems.size()), tags[i]));
		}
	}

}
