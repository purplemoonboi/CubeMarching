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
		FillMode fillMode 
	)
	{
		
	}

	DX12PipelineStateObject::DX12PipelineStateObject
	(
		GraphicsContext* graphicsContext,
		Shader* vertexShader,
		Shader* pixelShader,
		const BufferLayout& layout,
		FillMode fillMode 
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


		InputLayout =
		{
			{	"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,       0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{	"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,       0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{	"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,          0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psoDesc.InputLayout = { InputLayout.data(), static_cast<UINT>(InputLayout.size()) };

		psoDesc.pRootSignature = dx12GraphicsContext->RootSignature.Get();

		auto vsSize = VertexShader->GetShader()->GetBufferSize();
		auto psSize = PixelShader->GetShader()->GetBufferSize();


		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(VertexShader->GetShader()->GetBufferPointer()),
			vsSize
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(PixelShader->GetShader()->GetBufferPointer()),
			psSize
		};

		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FillMode = (fillMode == FillMode::Opaque) ? D3D12_FILL_MODE_SOLID : D3D12_FILL_MODE_WIREFRAME;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = dx12GraphicsContext->GetBackBufferFormat();
		psoDesc.SampleDesc.Count = dx12GraphicsContext->GetMsaaState() ? 4 : 1;
		psoDesc.SampleDesc.Quality = dx12GraphicsContext->GetMsaaState() ? (dx12GraphicsContext->GetMsaaQaulity() - 1) : 0;
		psoDesc.DSVFormat = dx12GraphicsContext->GetDepthStencilFormat();


		HRESULT creationResult = S_OK;
		creationResult  = dx12GraphicsContext->Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&Pso));
		THROW_ON_FAILURE(creationResult);
	}

	DX12PipelineStateObject::DX12PipelineStateObject(GraphicsContext* graphicsContext, Shader* computeShader)
	{

		const auto dx12GraphicsContext = dynamic_cast<DX12GraphicsContext*>(graphicsContext);
		const auto dx12ComputeShader = dynamic_cast<DX12Shader*>(computeShader);

		D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
		computePsoDesc.pRootSignature = dx12GraphicsContext->ComputeRootSignature.Get();
		computePsoDesc.CS =
		{
			reinterpret_cast<BYTE*>(dx12ComputeShader->GetComPointer()->GetBufferPointer()),
			dx12ComputeShader->GetComPointer()->GetBufferSize()
		};

		computePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		THROW_ON_FAILURE
		(
			dx12GraphicsContext->Device->CreateComputePipelineState
			(
				&computePsoDesc, IID_PPV_ARGS(&Pso)
			)
		);
	}

	DX12PipelineStateObject::~DX12PipelineStateObject()
	{

	}

	

}
