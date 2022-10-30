#include "DX12RenderingApi.h"
#include "DirectX12.h"
#include "DX12ShaderUtils.h"

#include "Platform/DirectX12/DX12GraphicsContext.h"
#include "Platform/DirectX12/DX12FrameBuffer.h"


#include "Framework/Core/Log/Log.h"

namespace DX12Framework
{
	DX12RenderingApi::~DX12RenderingApi()
	{
		if(GraphicsContext)
		{
			delete GraphicsContext;
			GraphicsContext = nullptr;
		}

		if(FrameBuffer)
		{
			delete FrameBuffer;
			FrameBuffer = nullptr;
		}
	}

	void DX12RenderingApi::Init()
	{
		GraphicsContext = nullptr;
		FrameBuffer = nullptr;
	}

	void DX12RenderingApi::InitD3D(HWND windowHandle, INT32 viewportWidth, INT32 viewportHeight)
	{
		/** builds our cbv descriptor etc */
		GraphicsContext = new DX12GraphicsContext(windowHandle, viewportWidth, viewportHeight);

		FrameBufferSpecifications fbs;
		fbs.Width = viewportWidth;
		fbs.Height = viewportHeight;

		IsResizing = true;
		FrameBuffer = new DX12FrameBuffer(fbs);
		FrameBuffer->Invalidate(GraphicsContext);
		IsResizing = false;

		THROW_ON_FAILURE(GraphicsContext->GraphicsCmdList->Reset(GraphicsContext->DirCmdListAlloc.Get(), nullptr));

		/** build constant buffer */
		BuildConstantBuffer(GraphicsContext->Device.Get(), GraphicsContext->ConstantBufferView(), 1, true);
		/** build the root signature */
		BuildRootSignature(GraphicsContext->Device.Get());
		/** build shader input and shader layout */
		BuildShaderInputAndLayout();
		/** build the box geometry */
		BuildBoxGeometry(GraphicsContext->Device.Get(), GraphicsContext->GraphicsCmdList.Get());
		/** build the pipeline state */
		BuildPSO(GraphicsContext->Device.Get(), GraphicsContext->GetBackBufferFormat(), GraphicsContext->GetDepthStencilFormat());

		HRESULT H = GraphicsContext->GraphicsCmdList->Close();
		THROW_ON_FAILURE(H);

		ID3D12CommandList* cmdList[] = { GraphicsContext->GraphicsCmdList.Get() };
		GraphicsContext->CommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

		GraphicsContext->FlushCommandQueue();


	}

	void DX12RenderingApi::SetViewport(INT32 x, INT32 y, INT32 width, INT32 height)
	{
		if(GraphicsContext != nullptr && FrameBuffer != nullptr)
		{
			CORE_TRACE("Buffer resize");
			FrameBuffer->SetViewportDimensions(width, height);
			FrameBuffer->Invalidate(GraphicsContext);
		}
	}

	void DX12RenderingApi::SetClearColour(const float colour[4])
	{

	}

	void DX12RenderingApi::Draw()
	{
		if (GraphicsContext != nullptr && !IsResizing)
		{
			// Reset the command allocator
			GraphicsContext->DirCmdListAlloc->Reset();




			// Reset the command list
			HRESULT cmdReset = GraphicsContext->GraphicsCmdList->Reset
			(
				GraphicsContext->DirCmdListAlloc.Get(),
				Pso.Get()
			);


			THROW_ON_FAILURE(cmdReset);



			// Reset the viewport and scissor rect whenever the command list is empty.
			GraphicsContext->GraphicsCmdList->RSSetViewports(1, &FrameBuffer->GetViewport());
			GraphicsContext->GraphicsCmdList->RSSetScissorRects(1, &FrameBuffer->GetScissorsRect());





			// Indicate there will be a transition made to the resource.
			GraphicsContext->GraphicsCmdList->ResourceBarrier
			(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition
				(
					GraphicsContext->CurrentBackBuffer(),
					D3D12_RESOURCE_STATE_PRESENT,
					D3D12_RESOURCE_STATE_RENDER_TARGET
				)
			);


			//Clear the back buffer 
			GraphicsContext->GraphicsCmdList->ClearRenderTargetView
			(
				GraphicsContext->CurrentBackBufferView(),
				DirectX::Colors::Aquamarine,
				0,
				nullptr
			);

			// Clear the depth buffer
			GraphicsContext->GraphicsCmdList->ClearDepthStencilView
			(
				GraphicsContext->DepthStencilView(),
				D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
				1.0f,
				0,
				0,
				nullptr
			);







			// Set the render targets descriptors
			GraphicsContext->GraphicsCmdList->OMSetRenderTargets
			(
				1,
				&GraphicsContext->CurrentBackBufferView(),
				true,
				&GraphicsContext->DepthStencilView()
			);






			ID3D12DescriptorHeap* descriptorHeaps[] = { GraphicsContext->CbvHeap.Get() };
			GraphicsContext->GraphicsCmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);






			GraphicsContext->GraphicsCmdList->SetGraphicsRootSignature(RootSignature.Get());





			GraphicsContext->GraphicsCmdList->IASetVertexBuffers(0, 1, &BoxGeo->VertexBufferView());
			GraphicsContext->GraphicsCmdList->IASetIndexBuffer(&BoxGeo->IndexBufferView());
			GraphicsContext->GraphicsCmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			GraphicsContext->GraphicsCmdList->SetGraphicsRootDescriptorTable(0, GraphicsContext->CbvHeap->GetGPUDescriptorHandleForHeapStart());


			GraphicsContext->GraphicsCmdList->DrawIndexedInstanced
			(
				BoxGeo->DrawArgs["box"].IndexCount,
				1,
				0, 
				0, 
				0
			);


			// Now instruct we have made the changes to the buffer
			GraphicsContext->GraphicsCmdList->ResourceBarrier
			(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition
				(
					GraphicsContext->CurrentBackBuffer(),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_PRESENT
				)
			);





			// We can now close the command list
			GraphicsContext->GraphicsCmdList->Close();





			ID3D12CommandList* cmdsLists[] = { GraphicsContext->GraphicsCmdList.Get() };
			GraphicsContext->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);


			THROW_ON_FAILURE(GraphicsContext->SwapChain->Present(0, 0));


			GraphicsContext->UpdateBackBufferIndex((GraphicsContext->GetBackBufferIndex() + 1) % SwapChainBufferCount);


			GraphicsContext->FlushCommandQueue();
		}
	}

	void DX12RenderingApi::Clear()
	{

	}

	void DX12RenderingApi::UpdateConstantBuffer(DirectX::XMMATRIX worldViewProj)
	{
		ObjectConstant objConstants;
		XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
		ObjectCB->CopyData(0, objConstants);
	}


	void DX12RenderingApi::BuildConstantBuffer
	(
		ID3D12Device* device,
		D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle,
		UINT count,
		bool isConstantBuffer
	)
	{

		ObjectCB = CreateRef<DX12UploadBuffer<ObjectConstant>>(device, count, isConstantBuffer);

		UINT objectCBBytes = DX12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));

		// Get the mapped virtual address located on the GPU
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = ObjectCB->Resource()->GetGPUVirtualAddress();

		// Offset to the ith object in the constant buffer
		INT32 boxCBufferIndex = 0;
		cbAddress += boxCBufferIndex * objectCBBytes;


		// Create a constant buffer view
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = DX12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));

		HRESULT h = device->GetDeviceRemovedReason();

		//THROW_ON_FAILURE(h);

		device->CreateConstantBufferView
		(
			&cbvDesc,
			cbvHandle
		);



	}

	void DX12RenderingApi::BuildRootSignature(ID3D12Device* device)
	{
		// Root parameter can be a table, root descriptors or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[1];

		// Create a single descriptor table of CRVs
		CD3DX12_DESCRIPTOR_RANGE cbvTable;
		cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc
		(
			1,
			slotRootParameter,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		ComPtr<ID3DBlob> serialisedRootSignature = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature
		(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serialisedRootSignature.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}

		THROW_ON_FAILURE(hr);

		THROW_ON_FAILURE(device->CreateRootSignature
		(
			0,
			serialisedRootSignature->GetBufferPointer(),
			serialisedRootSignature->GetBufferSize(),
			IID_PPV_ARGS(&RootSignature)
		));

	}

	void DX12RenderingApi::BuildShaderInputAndLayout()
	{
		HRESULT hr = S_OK;
														   
		std::wstring vsFilePath = L"assets\\shaders\\color.hlsl";
		std::wstring psFilePath = L"assets\\shaders\\color.hlsl";

		mvsByteCode = DX12ShaderUtils::CompileShader(vsFilePath, nullptr, "VS", "vs_5_0");
		mpsByteCode = DX12ShaderUtils::CompileShader(psFilePath, nullptr, "PS", "ps_5_0");

		InputLayout =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}

	void DX12RenderingApi::BuildBoxGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* graphicsCmdList)
	{

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

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);
		UINT oo = 1;

		BoxGeo = CreateRef<MeshGeometry>();
		BoxGeo->Name = "boxGeo";

		// Reserve memory and copy the vertices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(vbByteSize, &BoxGeo->VertexBufferCPU));
		CopyMemory(BoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		// Reserve memory and copy the indices into our CPU buffer
		THROW_ON_FAILURE(D3DCreateBlob(ibByteSize, &BoxGeo->IndexBufferCPU));
		CopyMemory(BoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		// Create the GPU vertex buffer
		BoxGeo->VertexBufferGPU = DX12BufferUtils::CreateDefaultBuffer(device,
			graphicsCmdList, vertices.data(), vbByteSize, BoxGeo->VertexBufferUploader);

		// Create the GPU index buffer
		BoxGeo->IndexBufferGPU = DX12BufferUtils::CreateDefaultBuffer(device,
			graphicsCmdList, indices.data(), ibByteSize, BoxGeo->IndexBufferUploader);


		BoxGeo->VertexByteStride = sizeof(Vertex);
		BoxGeo->VertexBufferByteSize = vbByteSize;
		BoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
		BoxGeo->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		BoxGeo->DrawArgs["box"] = submesh;

	}

	void DX12RenderingApi::BuildPSO
	(
		ID3D12Device* device,
		DXGI_FORMAT backbufferFormat, 
		DXGI_FORMAT depthStencilFormat
	)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psoDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };

		psoDesc.pRootSignature = RootSignature.Get();

		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
			mvsByteCode->GetBufferSize()
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
			mpsByteCode->GetBufferSize()
		};

		/** set to default states */
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

		psoDesc.SampleMask = UINT_MAX;

		/** it should expect triangle topology */
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		/** theres just one render target for now (output window) */
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = backbufferFormat;
		psoDesc.SampleDesc.Count = false ? 4 : 1;
		psoDesc.SampleDesc.Quality = false ? (4 - 1) : 0;
		psoDesc.DSVFormat = depthStencilFormat;

		HRESULT H = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&Pso));
		THROW_ON_FAILURE(H);
	}
}
