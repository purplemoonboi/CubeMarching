#pragma once
#include "Framework/Renderer/Api/RendererAPI.h"


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

		void Clean() override;

		void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) override;

		void PreInit() override;
		void PostInit() override;

		void OnBeginRender() override;
		void OnEndRender() override;

		void Flush() override;


		void Draw(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer) override;
		void DrawIndexed(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer) override;
		void DrawInstanced(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer, UINT32 instanceCount) override;
		void DrawIndexedInstanced(RenderPipeline* renderPipeline, VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, UINT32 instanceCount) override;


	private:


	};

}

