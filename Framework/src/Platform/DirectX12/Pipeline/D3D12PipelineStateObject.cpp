#include "D3D12PipelineStateObject.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

#include "Framework/Core/Log/Log.h"
#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"

namespace Engine
{
	D3D12PipelineStateObject::D3D12PipelineStateObject
	(
		GraphicsContext* graphicsContext, 
		const std::string& vertexShader,
		const std::string& pixelShader, 
		const BufferLayout& layout,
		FillMode fillMode 
	)
	{
		
	}

	D3D12PipelineStateObject::D3D12PipelineStateObject
	(
		GraphicsContext* graphicsContext,
		Shader* vertexShader,
		Shader* pixelShader,
		const BufferLayout& layout,
		FillMode fillMode 
	)
	{
		/** cast to DX12 graphics context */
		auto dx12GraphicsContext = dynamic_cast<D3D12Context*>(graphicsContext);
		CORE_ASSERT(dx12GraphicsContext, "Failed to cast graphics context into DirectX 12 context...");


		/** pso description */
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;


		/** cast to DX12 shaders */
		auto VertexShader = dynamic_cast<D3D12Shader*>(vertexShader);
		auto PixelShader = dynamic_cast<D3D12Shader*>(pixelShader);

		CORE_ASSERT(VertexShader, "Cast failed!");
		CORE_ASSERT(PixelShader, "Cast failed!");

		std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;

		InputLayout =
		{
			{	"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,       0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{	"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,       0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{	"TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,       0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{	"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,          0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = dx12GraphicsContext->GetMsaaState() ? 4 : 1;
		psoDesc.SampleDesc.Quality = dx12GraphicsContext->GetMsaaState() ? (dx12GraphicsContext->GetMsaaQaulity() - 1) : 0;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;


		HRESULT creationResult = S_OK;
		creationResult  = dx12GraphicsContext->Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&Pso));
		THROW_ON_FAILURE(creationResult);
	}

	D3D12PipelineStateObject::D3D12PipelineStateObject
	(
		ComputeApi* computeContext, 
		Shader* computeShader,
		ComPtr<ID3D12RootSignature> rootSignature

	)
	{

		const auto d3d12Shader = dynamic_cast<D3D12Shader*>(computeShader);
		const auto d3d12ComputeApi = dynamic_cast<D3D12ComputeApi*>(computeContext);

		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = rootSignature.Get();
		desc.CS =
		{
			reinterpret_cast<BYTE*>(d3d12Shader->GetShader()->GetBufferPointer()),
			d3d12Shader->GetShader()->GetBufferSize()
		};
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		const HRESULT csPipelineState = d3d12ComputeApi->Context->Device->CreateComputePipelineState
		(
			&desc,
			IID_PPV_ARGS(&Pso)
		);
		THROW_ON_FAILURE(csPipelineState);
		
	}

	D3D12PipelineStateObject::~D3D12PipelineStateObject()
	{

	}

	

}
