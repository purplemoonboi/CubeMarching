#include "Renderer3D.h"
#include "RenderInstruction.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Geometry.h"
#include "PipelineStateObject.h"

#include "Platform/DirectX12/DX12FrameResource.h"


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
		std::unordered_map<std::string, RefPointer<PipelineStateObject>> PSOs;


		// Struct capturing performance
		Renderer3D::RenderingStats RendererStats;
	};

	static RendererData RenderData;


	void Renderer3D::Init()
	{
		/** get the graphics context */
		const auto api = RenderInstruction::GetApiPtr();

		/** build and compile shaders */
		RenderData.VertexShader = Shader::Create(L"assets\\shaders\\color.hlsl", "VS", "vs_5_1");
		RenderData.PixelShader = Shader::Create(L"assets\\shaders\\color.hlsl", "VS", "ps_5_1");


		/** build the pipeline state objects */
		RenderData.PSOs["opaque"] = PipelineStateObject::Create
		(
			api->GetGraphicsContext(), 
			RenderData.VertexShader.get(), 
			RenderData.PixelShader.get(), 
			RenderData.Geometry->VertexBuffer->GetLayout()
		);

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

		RenderData.Geometry->VertexBuffer = VertexBuffer::Create(api->GetGraphicsContext(), static_cast<UINT>(vertices.size()) * sizeof(Vertex));
		/** create input layout */
		RenderData.Geometry->VertexBuffer->SetLayout
		({
			 {  "POSITION" , ShaderDataType::Float3	,	0},
			 {  "COLOR"	    , ShaderDataType::Float4,  12	}
			}
		);

		SubGeometry box;
		box.IndexCount = (UINT)indices.size();
		box.StartIndexLocation = 0;
		box.BaseVertexLocation = 0;

		RenderData.Geometry->DrawArgs["Box"] = box;

	}

	void Renderer3D::Shutdown()
	{
	}


	void Renderer3D::BeginScene(MainCamera& cam)
	{
		//Update the world matrixs
		RenderData.ConstantBuffer->Update(cam);


		// We set the colour to blue in the api
		RenderInstruction::SetClearColour(nullptr);
	}

	void Renderer3D::EndScene()
	{
		RenderInstruction::DrawIndexed(RenderData.Geometry);
		RenderInstruction::Flush();

	}

	void Renderer3D::DrawDemoBox()
	{
		/** for now we're going to force draw a cube. Not efficient will implement improved code soon! */
		RenderInstruction::DrawDemoScene();
	}



}
