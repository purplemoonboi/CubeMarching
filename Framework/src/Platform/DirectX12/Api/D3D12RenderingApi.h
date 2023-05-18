#pragma once
#include "Framework/Renderer/Api/RendererAPI.h"

#include "D3D12Context.h"
#include "Platform/DirectX12/Buffers/D3D12FrameBuffer.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"
#include "Platform/DirectX12/Allocator/D3D12HeapManager.h"
#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Resources/D3D12ResourceManager.h"
#include "Platform/DirectX12/Textures/D3D12RenderTarget.h"

namespace Foundation
{
	class D3D12RootSignature;

	struct WorldSettings;

	struct ObjectConstant;

	constexpr UINT GBufferTextureCount = 5;

	class D3D12RenderingApi : public RendererAPI
	{
	public:

		virtual ~D3D12RenderingApi() ;

		void Init(GraphicsContext* context, INT32 viewportWidth, INT32 viewportHeight) override;

		void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) override;

		void BindDepthPass() override;

		void DrawTerrainGeometry(PipelineStateObject* pso, RenderItem* terrain) override;

		void BindScenePass() override;
		void DrawSceneStaticGeometry(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems) override;

		void BindLightingPass() override;

		void BindPostProcessingPass() override;

		void Flush() override;
		void PreInit() override;
		void PostInit() override;

		void DrawIndexed(const RefPointer<VertexArray>& vertexArray, INT32 indexCount = 0) override {}
		void DrawIndexed(const ScopePointer<MeshGeometry>& geometry, INT32 indexCount = 0) override {}

		void PreRender
		(
			const std::vector<RenderItem*>& items, const std::vector<Material*>& materials,
			RenderItem* terrain,
			const WorldSettings& settings,
			const MainCamera& camera,
			float deltaTime,
			float elapsedTime,
			bool wireframe
		) override;

		void PostRender() override;

		void OnBeginRender() override;

		void OnEndRender() override;

		[[nodiscard]] GraphicsContext* GetGraphicsContext() const override { return Context; }

		[[nodiscard]] FrameBuffer* GetFrameBuffer() const override { return FrameBuffer.get(); };


		[[nodiscard]] D3D12FrameResource* GetCurrentFrameResource() const { return Frames[FrameIndex].get(); }

		[[nodiscard]] RenderTarget* GetSceneTexture() const override { return RenderTargets[(INT8)RenderLayer::Albedo].get(); }

	private:

		

		std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

		// Buffer for uploading shader constants.
		ScopePointer<D3D12ResourceBuffer> UploadBuffer;

		// All of our frame resources.
		std::array<ScopePointer<D3D12FrameResource>, FRAMES_IN_FLIGHT> Frames;
		UINT32 FrameIndex = 0;

		

		// Defines shader inputs
		ComPtr<ID3D12RootSignature> RootSignature;

		// A pointer to the graphics context
		D3D12Context* Context = nullptr;
		// A unique pointer to the frame buffer
		ScopePointer<D3D12FrameBuffer> FrameBuffer = nullptr;
	
		// A unique pointer to the Api's memory allocator class.
		ScopePointer<D3D12HeapManager> HeapManager = nullptr;

		std::array<ScopePointer<D3D12RenderTarget>, GBufferTextureCount> RenderTargets;
		//std::array<ScopePointer<D3D12Shader>, GBufferTextureCount> CoreShaders;

	};

}

