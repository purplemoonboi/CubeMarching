#pragma once
#include "Framework/Renderer/Api/RendererAPI.h"
#include "Platform/DirectX12/Constants/D3D12GlobalConstants.h"


namespace Foundation::Graphics::D3D12
{
	class D3D12Context;
	class D3D12RenderFrame;
	class D3D12ResourceBuffer;
	class D3D12RenderTarget;
	class D3D12FrameBuffer;
	class D3D12DescriptorHeap;
	class D3D12RenderPipeline;
	class D3D12RootSignature;


	class D3D12RenderingApi : public RendererAPI
	{
	public:
		explicit D3D12RenderingApi();
		DISABLE_COPY_AND_MOVE(D3D12RenderingApi);

		~D3D12RenderingApi() override;

		void Init(GraphicsContext* context) override;
		void Clean() override;

		void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) override;

		void PreInit() override;
		void PostInit() override;

		void OnBeginRender() override;
		void OnEndRender() override;

		void Flush() override;

		void DrawSceneStaticGeometry(RenderPipeline* pso, const std::vector<RenderItem*>& renderItems) override;
		void DrawIndexed(const ScopePointer<MeshGeometry>& geometry, INT32 indexCount = 0) override {}


	private:
		// A pointer to the graphics context
		D3D12Context* Context;

	};

}

