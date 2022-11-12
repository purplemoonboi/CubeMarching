#include "DX12PipelineStateObject.h"
#include "DX12Shader.h"
#include "Framework/Core/Log/Log.h"

#include "Platform/DirectX12/DX12GraphicsContext.h"

namespace Engine
{
	DX12PipelineStateObject::DX12PipelineStateObject
	(
		GraphicsContext* graphicsContext, 
		const std::string& vertexShader,
		const std::string& pixelShader, 
		const BufferLayout& layout, 
		UINT backBufferFormat, 
		UINT depthStencilFormal
	)
	{
		
	}

	DX12PipelineStateObject::DX12PipelineStateObject
	(
		GraphicsContext* graphicsContext,
		Shader* vertexShader,
		Shader* pixelShader,
		const BufferLayout& layout,
		UINT backBufferFormat,
		UINT depthStencilFormal
	)
	{
		/** cast to DX12 graphics context */
		auto dx12GraphicsContext = dynamic_cast<DX12GraphicsContext*>(graphicsContext);
		CORE_ASSERT(dx12GraphicsContext, "Failed to cast graphics context into DirectX 12 context...");


		/** pso description */
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;

		/** cast to DX12 shaders */
		auto VertexShader = dynamic_cast<DX12Shader*>(vertexShader);
		auto PixelShader = dynamic_cast<DX12Shader*>(pixelShader);

		CORE_ASSERT(VertexShader, "Cast failed!");
		CORE_ASSERT(PixelShader, "Cast failed!");

		std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;

		/** build shader layout */
		const BufferElement& vertex = layout.GetElements()[0];
		const BufferElement& colour = layout.GetElements()[1];

		InputLayout =
		{
			{vertex.Name.c_str(), 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{  colour.Name.c_str(), 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psoDesc.InputLayout = { InputLayout.data(), static_cast<UINT>(InputLayout.size()) };
		psoDesc.pRootSignature = dx12GraphicsContext->RootSignature.Get();
		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(VertexShader->GetComPointer()->GetBufferPointer()),
			VertexShader->GetComPointer()->GetBufferSize()
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(PixelShader->GetComPointer()->GetBufferPointer()),
			PixelShader->GetComPointer()->GetBufferSize()
		};
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = static_cast<DXGI_FORMAT>(backBufferFormat);
		//psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		//psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.DSVFormat = psoDesc.RTVFormats[0] = static_cast<DXGI_FORMAT>(depthStencilFormal);

		HRESULT h = dx12GraphicsContext->Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&Pso));
		HRESULT cb = dx12GraphicsContext->Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(h);

		dx12GraphicsContext->CurrentPso = Pso.Get();
	}

	DX12PipelineStateObject::~DX12PipelineStateObject()
	{

	}

	

}
