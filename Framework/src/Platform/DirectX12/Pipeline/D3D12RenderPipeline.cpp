#include "D3D12RenderPipeline.h"
#include "Framework/Core/Log/Log.h"

#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Shaders/D3D12Shader.h"
#include "Platform/DirectX12/Compute/D3D12ComputeApi.h"

namespace Foundation::Graphics::D3D12
{

	D3D12RenderPipeline::~D3D12RenderPipeline()
	{}

	void D3D12RenderPipeline::Bind()
	{

	}

	void D3D12RenderPipeline::Destroy()
	{

	}

	void D3D12RenderPipeline::SetVertexShader(const std::string_view& name, Shader* shader)
	{
		const auto pD3D12Shader = dynamic_cast<D3D12Shader*>(shader);

		ShaderPipeline[0] = std::move(*pD3D12Shader);

	}

	void D3D12RenderPipeline::SetPixelShader(const std::string_view& name, Shader* shader)
	{
	}

	void D3D12RenderPipeline::SetHullShader(const std::string_view& name, Shader* shader)
	{
	}

	void D3D12RenderPipeline::SetDomainShader(const std::string_view& name, Shader* shader)
	{
	}

	void D3D12RenderPipeline::SetGeometryShader(const std::string_view& name, Shader* shader)
	{
	}

	void D3D12RenderPipeline::InitialiseRoot(const std::vector<PipelineInputDesc>& pipelineIn)
	{
	}

	void D3D12RenderPipeline::LoadShader(const ShaderDesc& desc)
	{
		if(ShaderLibrary::Get(desc.Name) == nullptr)
		{
			auto shader = ShaderLibrary::Load(desc);
		}

	}

	D3D12RenderPipeline::D3D12RenderPipeline(PipelineDesc& desc)
		:
		RenderPipeline(desc)
	{

		InitialiseRoot(desc.InputDesc);

		

		std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
		/*InputLayout =
		{
			{	"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,       0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{	"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,       0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{	"TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,       0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{	"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,          0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};*/
		for(auto& element : desc.Layout)
		{
			D3D12_INPUT_ELEMENT_DESC a = { element.Name,  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
			InputLayout.emplace_back();
		}

		/** pso description */
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psoDesc.InputLayout = { InputLayout.data(), static_cast<UINT>(InputLayout.size()) };

		psoDesc.pRootSignature = RootSignature.Get();


		for (auto* s : desc.s)
		{
			if (s != nullptr)
			{
				D3D12Shader* shaders = dynamic_cast<D3D12Shader*>(s);

				if(shaders != nullptr)
				{
					ID3DBlob* shaderData = shaders->GetComPointer();
					LPVOID pBuf = static_cast<BYTE*>(shaders->GetShader()->GetBufferPointer());
					SIZE_T size = shaders->GetShader()->GetBufferSize();

					switch (shaders->GetShaderType())
					{
					case EShaderType::VS:
						psoDesc.VS = { pBuf, size };
						break;
					case EShaderType::HS:
						psoDesc.HS = { pBuf, size };
						break;
					case EShaderType::DS:
						psoDesc.DS = { pBuf, size };
						break;
					case EShaderType::GS:
						psoDesc.GS = { pBuf, size };
						break;
					case EShaderType::PS:
						psoDesc.PS = { pBuf, size };
						break;
					}
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
		psoDesc.SampleDesc.Count = GetMSAAState() ? 4 : 1;
		psoDesc.SampleDesc.Quality = GetMSAAState() ? (GetMSAAQuality() - 1) : 0;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		HRESULT hr = S_OK;
		hr = GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&Pso));
		THROW_ON_FAILURE(hr);
	}

	D3D12RenderPipeline::D3D12RenderPipeline(D3D12RenderPipeline&& desc)
	{
	}

	auto D3D12RenderPipeline::operator=(D3D12RenderPipeline&& rhs) noexcept -> D3D12RenderPipeline&
	{
	}


	/*D3D12RenderPipeline::D3D12RenderPipeline
	(
		Shader* computeShader,
		ComPtr<ID3D12RootSignature> rootSignature

	)
	{

		const auto d3d12Shader = dynamic_cast<D3D12Shader*>(computeShader);
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
		THROW_ON_FAILURE(csPipelineState);
		
	}*/


	void D3D12RenderPipeline::InitialiseRoot(const PipelineInputDesc& pipelineIn)
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

		hr = GetDevice()->CreateRootSignature
		(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&RootSignature)
		);
		THROW_ON_FAILURE(hr);
	}



}
