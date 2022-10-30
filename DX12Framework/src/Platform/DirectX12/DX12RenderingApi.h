#pragma once
#include <intsafe.h>
#include "DirectX12.h"
#include "Framework/Renderer/RendererAPI.h"
#include "DX12Buffer.h"

namespace DX12Framework
{

	class DX12GraphicsContext;
	class DX12FrameBuffer;

	class DX12RenderingApi : public RendererAPI
	{
	public:
		virtual ~DX12RenderingApi();


		void Init() override;

		void InitD3D(HWND windowHandle, INT32 viewportWidth, INT32 viewportHeight) override;

		void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) override;

		void SetClearColour(const float colour[4]) override;

		void Draw() override;

		void Clear() override;

		void UpdateConstantBuffer(DirectX::XMMATRIX worldViewProj);

		void SetIsResizing(bool value) {IsResizing = value;}
		bool IsBufferResizing() const { return IsResizing; }

		void BuildConstantBuffer
		(
			ID3D12Device* device,
			D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle,
			UINT count,
			bool isConstantBuffer = true
		);

		// @brief Defines the resources the shader programs should expect.
		void BuildRootSignature(ID3D12Device* device);

		void BuildShaderInputAndLayout();

		void BuildBoxGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* graphicsCmdList);

		void BuildPSO(ID3D12Device* device, DXGI_FORMAT backbufferFormat, DXGI_FORMAT depthStencilFormat);

	private:

		DX12GraphicsContext* GraphicsContext = nullptr;

		DX12FrameBuffer* FrameBuffer = nullptr;

		bool IsResizing;

		DX12ConstantBuffer* CBuffer = nullptr;





		ComPtr<ID3DBlob> mvsByteCode;
		ComPtr<ID3DBlob> mpsByteCode;
		ComPtr<ID3D12RootSignature> RootSignature;
		ComPtr<ID3D12PipelineState> Pso;
		RefPointer<DX12UploadBuffer<ObjectConstant>> ObjectCB = nullptr;

		std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
		RefPointer<MeshGeometry> BoxGeo;

	};

}

