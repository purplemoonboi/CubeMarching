#include "D3D12PipelineStateObject.h"
#include "Framework/Core/Log/Log.h"

#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Core/D3D12Core.h"

#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"

namespace Foundation::Graphics::D3D12
{

	D3D12PipelineStateObject::~D3D12PipelineStateObject()
	{

	}

	D3D12PipelineStateObject::D3D12PipelineStateObject(D3D12Context* context, const PipelineResourceDesc& args, const PipelineDesc& desc)
	{

		CORE_ASSERT(context, "Failed to cast graphics context into DirectX 12 context...");
		InitialiseRoot(pDevice.Get(), args);

		/** pso description */
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;

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

		psoDesc.pRootSignature = RootSignature.Get();

		D3D12Shader* shaders;

		for (auto* s : desc.GetShaders())
		{
			if (s != nullptr)
			{
				shaders = dynamic_cast<D3D12Shader*>(s);
				LPVOID pBuf = reinterpret_cast<BYTE*>(shaders->GetShader()->GetBufferPointer());
				SIZE_T size = shaders->GetShader()->GetBufferSize();

				switch (shaders->GetShaderType())
				{
				case ShaderType::VS:
					psoDesc.VS = {pBuf, size};
					break;
				case ShaderType::HS:
					psoDesc.HS = { pBuf, size };
					break;
				case ShaderType::DS:
					psoDesc.DS = { pBuf, size };
					break;
				case ShaderType::GS:
					psoDesc.GS = { pBuf, size };
					break;
				case ShaderType::PS:
					psoDesc.PS = { pBuf, size };
					break;
				}
			}
		}

		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FillMode = (desc.Fill == FillMode::Opaque) ? D3D12_FILL_MODE_SOLID : D3D12_FILL_MODE_WIREFRAME;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = context->GetMsaaState() ? 4 : 1;
		psoDesc.SampleDesc.Quality = context->GetMsaaState() ? (context->GetMsaaQaulity() - 1) : 0;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		HRESULT hr = S_OK;
		hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&Pso));
		THROW_ON_FAILURE(hr);
	}

	D3D12PipelineStateObject::D3D12PipelineStateObject
	(
		D3D12Context* graphicsContext,
		D3D12Shader* vertexShader,
		D3D12Shader* pixelShader,
		FillMode fillMode 
	)
	{
		/** cast to DX12 graphics context */
		
		CORE_ASSERT(graphicsContext, "Failed to cast graphics context into DirectX 12 context...");


		/** pso description */
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;


		CORE_ASSERT(vertexShader, "Cast failed!");
		CORE_ASSERT(pixelShader, "Cast failed!");

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

		psoDesc.pRootSignature = RootSignature.Get();

		auto vsSize = vertexShader->GetShader()->GetBufferSize();
		auto psSize = pixelShader->GetShader()->GetBufferSize();


		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(vertexShader->GetShader()->GetBufferPointer()),
			vsSize
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(pixelShader->GetShader()->GetBufferPointer()),
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
		psoDesc.SampleDesc.Count = graphicsContext->GetMsaaState() ? 4 : 1;
		psoDesc.SampleDesc.Quality = graphicsContext->GetMsaaState() ? (graphicsContext->GetMsaaQaulity() - 1) : 0;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;


		HRESULT creationResult = S_OK;
		creationResult  = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&Pso));
		THROW_ON_FAILURE(creationResult);
	}

	D3D12PipelineStateObject::D3D12PipelineStateObject
	(
		ComputeApi* computeContext, 
		Shader* computeShader,
		ComPtr<ID3D12RootSignature> rootSignature

	)
	{

		/*const auto d3d12Shader = dynamic_cast<D3D12Shader*>(computeShader);
		const auto d3d12ComputeApi = dynamic_cast<Foundation::Compute::D3D12::D3D12ComputeApi*>(computeContext);

		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = rootSignature.Get();
		
		desc.CS =
		{
			reinterpret_cast<BYTE*>(d3d12Shader->GetShader()->GetBufferPointer()),
			d3d12Shader->GetShader()->GetBufferSize()
		};
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		const HRESULT csPipelineState = pDevice->CreateComputePipelineState
		(
			&desc,
			IID_PPV_ARGS(&Pso)
		);
		THROW_ON_FAILURE(csPipelineState);*/
		
	}

	void D3D12PipelineStateObject::InitialiseRoot(ID3D12Device* device, const PipelineResourceDesc& desc)
	{
		CD3DX12_DESCRIPTOR_RANGE textureTable0;
		textureTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0); // register t0

		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[4];

		slotRootParameter[0].InitAsConstantBufferView(0);// register b0
		slotRootParameter[1].InitAsConstantBufferView(1);// register b1
		slotRootParameter[2].InitAsConstantBufferView(2);// register b2
		slotRootParameter[3].InitAsDescriptorTable(1, &textureTable0, D3D12_SHADER_VISIBILITY_PIXEL);

		const auto samplers = GetStaticSamplers();

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc
		(
			4, slotRootParameter,
			samplers.size(), samplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature
		(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		THROW_ON_FAILURE(hr);

		hr = device->CreateRootSignature
		(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&RootSignature)
		);
		THROW_ON_FAILURE(hr);
	}



}
