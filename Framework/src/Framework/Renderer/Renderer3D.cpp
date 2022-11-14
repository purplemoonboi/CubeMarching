#include "Renderer3D.h"
#include "RenderInstruction.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Geometry.h"
#include "PipelineStateObject.h"

#include "Platform/DirectX12/DX12FrameResource.h"
#include "Platform/DirectX12/DX12RenderingApi.h"

namespace Engine
{

	
	struct RendererData
	{

		// Constant data
		static const UINT32 MaxTriangles = 100000;
		static const UINT32 MaxVertices = MaxTriangles * 4;
		static const UINT32 MaxIndices = MaxTriangles * 6;
		static const UINT32 MaxTextureSlots = 32;

		RefPointer<VertexArray> VertexArray;
		RefPointer<Shader> VertexShader;
		RefPointer<Shader> PixelShader;

		RefPointer<Geometry> Geometry;
		RefPointer<UploadBuffer> ConstantBuffer;
		RefPointer<PipelineStateObject> Pso;
		std::unordered_map<std::string, RefPointer<PipelineStateObject>> PSOs;


		// Struct capturing performance
		Renderer3D::RenderingStats RendererStats;
	};

	static RendererData RenderData;


	void Renderer3D::Init()
	{
		/** get the graphics context */
		const auto api = RenderInstruction::GetApiPtr();

		/** for apis such as DirectX12 and Vulkan, we need reset the command list before submitting instructions */
		api->ResetCommandList();


		/** build and compile shaders */
		RenderData.VertexShader = Shader::Create(L"assets\\shaders\\color.hlsl", "VS", "vs_5_0");
		RenderData.PixelShader = Shader::Create(L"assets\\shaders\\color.hlsl", "PS", "ps_5_0");

		RenderData.ConstantBuffer = UploadBuffer::Create(api->GetGraphicsContext(), 1, true);

		/** build geometry */
		std::array<Vertex, 8> vertices =
		{
			Vertex({ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::White) }),
			Vertex({ DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Black) }),
			Vertex({ DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) }),
			Vertex({ DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Green) }),
			Vertex({ DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Blue) }),
			Vertex({ DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) }),
			Vertex({ DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Cyan) }),
			Vertex({ DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Magenta) })
		};

		std::array<std::uint16_t, 36> indices =
		{
			// front face
			0, 1, 2,
			0, 2, 3,

			// back face
			4, 6, 5,
			4, 7, 6,

			// left face
			4, 5, 1,
			4, 1, 0,

			// right face
			3, 2, 6,
			3, 6, 7,

			// top face
			1, 5, 6,
			1, 6, 2,

			// bottom face
			4, 0, 3,
			4, 3, 7
		};

		RenderData.Geometry = Geometry::Create("Box");


		/** create vertex buffer and set the layout*/
		const UINT64 vbSizeInBytes = static_cast<UINT>(vertices.size()) * sizeof(Vertex);
		const UINT64 ibSizeInBytes = sizeof(UINT16) * static_cast<UINT>(indices.size());

		RenderData.Geometry->VertexBuffer = VertexBuffer::Create(api->GetGraphicsContext(), vertices.data(), vbSizeInBytes);
		RenderData.Geometry->VertexBuffer->SetLayout
		({
			 {  "POSITION" , ShaderDataType::Float3	,	0},
			 {  "COLOR"	    , ShaderDataType::Float4,  12	}
			}
		);

		RenderData.Geometry->IndexBuffer = IndexBuffer::Create(api->GetGraphicsContext(), indices.data(), ibSizeInBytes, indices.size());

		SubGeometry box;
		box.IndexCount = (UINT)indices.size();
		box.StartIndexLocation = 0;
		box.BaseVertexLocation = 0;

		RenderData.Geometry->DrawArgs["Box"] = box;



		/** create the pso */

		RenderData.Pso = PipelineStateObject::Create
		(
			api->GetGraphicsContext(),
			RenderData.VertexShader.get(),
			RenderData.PixelShader.get(),
			RenderData.Geometry->VertexBuffer->GetLayout()
		);


		/** build the pipeline state objects */
		RenderData.PSOs.emplace("opaque", RenderData.Pso);

		/** tell the GPU to execute the init commands and wait until we're finished */
		api->ExecCommandList();
	}

	void Renderer3D::Shutdown()
	{
	}


	void Renderer3D::BeginScene(MainCamera& cam)
	{
		//Update the world matrixs
		RenderData.ConstantBuffer->Update(cam);

		// Render the geometry and use the current Pso settings (Ignore the count (36) for now.)
		RenderInstruction::DrawIndexed(RenderData.Geometry, 36, RenderData.Pso.get());


		// We set the colour to blue in the api
	}

	void Renderer3D::EndScene()
	{

	}

	void Renderer3D::DrawDemoBox()
	{
		/** for now we're going to force draw a cube. Not efficient will implement improved code soon! */
	}

	//void ShapesApp::BuildShapeGeometry()
	//{
	//	GeometryGenerator geoGen;
	//	GeometryGenerator::MeshData box = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 3);
	//	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	//	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	//	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	//	//
	//	// We are concatenating all the geometry into one big vertex/index buffer.  So
	//	// define the regions in the buffer each submesh covers.
	//	//

	//	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	//	UINT boxVertexOffset = 0;
	//	UINT gridVertexOffset = (UINT)box.Vertices.size();
	//	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	//	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();

	//	// Cache the starting index for each object in the concatenated index buffer.
	//	UINT boxIndexOffset = 0;
	//	UINT gridIndexOffset = (UINT)box.Indices32.size();
	//	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	//	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();

	//	// Define the SubmeshGeometry that cover different 
	//	// regions of the vertex/index buffers.

	//	SubmeshGeometry boxSubmesh;
	//	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	//	boxSubmesh.StartIndexLocation = boxIndexOffset;
	//	boxSubmesh.BaseVertexLocation = boxVertexOffset;

	//	SubmeshGeometry gridSubmesh;
	//	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	//	gridSubmesh.StartIndexLocation = gridIndexOffset;
	//	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	//	SubmeshGeometry sphereSubmesh;
	//	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	//	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	//	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	//	SubmeshGeometry cylinderSubmesh;
	//	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
	//	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	//	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

	//	//
	//	// Extract the vertex elements we are interested in and pack the
	//	// vertices of all the meshes into one vertex buffer.
	//	//

	//	auto totalVertexCount =
	//		box.Vertices.size() +
	//		grid.Vertices.size() +
	//		sphere.Vertices.size() +
	//		cylinder.Vertices.size();

	//	std::vector<Vertex> vertices(totalVertexCount);

	//	UINT k = 0;
	//	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	//	{
	//		vertices[k].Pos = box.Vertices[i].Position;
	//		vertices[k].Color = XMFLOAT4(DirectX::Colors::DarkGreen);
	//	}

	//	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	//	{
	//		vertices[k].Pos = grid.Vertices[i].Position;
	//		vertices[k].Color = XMFLOAT4(DirectX::Colors::ForestGreen);
	//	}

	//	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	//	{
	//		vertices[k].Pos = sphere.Vertices[i].Position;
	//		vertices[k].Color = XMFLOAT4(DirectX::Colors::Crimson);
	//	}

	//	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	//	{
	//		vertices[k].Pos = cylinder.Vertices[i].Position;
	//		vertices[k].Color = XMFLOAT4(DirectX::Colors::SteelBlue);
	//	}

	//	std::vector<std::uint16_t> indices;
	//	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	//	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	//	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	//	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));

	//	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	//	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	//	auto geo = std::make_unique<MeshGeometry>();
	//	geo->Name = "shapeGeo";

	//	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	//	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	//	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	//	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	//	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
	//		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	//	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
	//		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	//	geo->VertexByteStride = sizeof(Vertex);
	//	geo->VertexBufferByteSize = vbByteSize;
	//	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	//	geo->IndexBufferByteSize = ibByteSize;

	//	geo->DrawArgs["box"] = boxSubmesh;
	//	geo->DrawArgs["grid"] = gridSubmesh;
	//	geo->DrawArgs["sphere"] = sphereSubmesh;
	//	geo->DrawArgs["cylinder"] = cylinderSubmesh;

	//	mGeometries[geo->Name] = std::move(geo);
	//}


	//void ShapesApp::BuildRenderItems()
	//{
	//	auto boxRitem = std::make_unique<RenderItem>();
	//	XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f));
	//	boxRitem->ObjCBIndex = 0;
	//	boxRitem->Geo = mGeometries["shapeGeo"].get();
	//	boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//	boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
	//	boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
	//	boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
	//	mAllRitems.push_back(std::move(boxRitem));

	//	auto gridRitem = std::make_unique<RenderItem>();
	//	gridRitem->World = MathHelper::Identity4x4();
	//	gridRitem->ObjCBIndex = 1;
	//	gridRitem->Geo = mGeometries["shapeGeo"].get();
	//	gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//	gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
	//	gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	//	gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
	//	mAllRitems.push_back(std::move(gridRitem));

	//	UINT objCBIndex = 2;
	//	for (int i = 0; i < 5; ++i)
	//	{
	//		auto leftCylRitem = std::make_unique<RenderItem>();
	//		auto rightCylRitem = std::make_unique<RenderItem>();
	//		auto leftSphereRitem = std::make_unique<RenderItem>();
	//		auto rightSphereRitem = std::make_unique<RenderItem>();

	//		XMMATRIX leftCylWorld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
	//		XMMATRIX rightCylWorld = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f);

	//		XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f);
	//		XMMATRIX rightSphereWorld = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f);

	//		XMStoreFloat4x4(&leftCylRitem->World, rightCylWorld);
	//		leftCylRitem->ObjCBIndex = objCBIndex++;
	//		leftCylRitem->Geo = mGeometries["shapeGeo"].get();
	//		leftCylRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//		leftCylRitem->IndexCount = leftCylRitem->Geo->DrawArgs["cylinder"].IndexCount;
	//		leftCylRitem->StartIndexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
	//		leftCylRitem->BaseVertexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;

	//		XMStoreFloat4x4(&rightCylRitem->World, leftCylWorld);
	//		rightCylRitem->ObjCBIndex = objCBIndex++;
	//		rightCylRitem->Geo = mGeometries["shapeGeo"].get();
	//		rightCylRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//		rightCylRitem->IndexCount = rightCylRitem->Geo->DrawArgs["cylinder"].IndexCount;
	//		rightCylRitem->StartIndexLocation = rightCylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
	//		rightCylRitem->BaseVertexLocation = rightCylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;

	//		XMStoreFloat4x4(&leftSphereRitem->World, leftSphereWorld);
	//		leftSphereRitem->ObjCBIndex = objCBIndex++;
	//		leftSphereRitem->Geo = mGeometries["shapeGeo"].get();
	//		leftSphereRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//		leftSphereRitem->IndexCount = leftSphereRitem->Geo->DrawArgs["sphere"].IndexCount;
	//		leftSphereRitem->StartIndexLocation = leftSphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
	//		leftSphereRitem->BaseVertexLocation = leftSphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;

	//		XMStoreFloat4x4(&rightSphereRitem->World, rightSphereWorld);
	//		rightSphereRitem->ObjCBIndex = objCBIndex++;
	//		rightSphereRitem->Geo = mGeometries["shapeGeo"].get();
	//		rightSphereRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//		rightSphereRitem->IndexCount = rightSphereRitem->Geo->DrawArgs["sphere"].IndexCount;
	//		rightSphereRitem->StartIndexLocation = rightSphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
	//		rightSphereRitem->BaseVertexLocation = rightSphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;

	//		mAllRitems.push_back(std::move(leftCylRitem));
	//		mAllRitems.push_back(std::move(rightCylRitem));
	//		mAllRitems.push_back(std::move(leftSphereRitem));
	//		mAllRitems.push_back(std::move(rightSphereRitem));
	//	}

	//	// All the render items are opaque.
	//	for (auto& e : mAllRitems)
	//		mOpaqueRitems.push_back(e.get());
	//}


	//UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	//auto objectCB = mCurrFrameResource->ObjectCB->Resource();

	//// For each render item...
	//for (size_t i = 0; i < ritems.size(); ++i)
	//{
	//	auto ri = ritems[i];

	//	cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
	//	cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
	//	cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

	//	// Offset to the CBV in the descriptor heap for this object and for this frame resource.
	//	UINT cbvIndex = mCurrFrameResourceIndex * (UINT)mOpaqueRitems.size() + ri->ObjCBIndex;
	//	auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
	//	cbvHandle.Offset(cbvIndex, mCbvSrvUavDescriptorSize);

	//	cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);

	//	cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	//}

}
