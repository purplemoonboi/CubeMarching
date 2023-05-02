#pragma once
#include "Framework/Renderer/Api/RendererAPI.h"

#include "D3D12Context.h"
#include "Platform/DirectX12/Buffers/D3D12FrameBuffer.h"
#include "Platform/DirectX12/Resources/D3D12FrameResource.h"
#include "Platform/DirectX12/Allocator/D3D12MemoryManager.h"
#include "Platform/DirectX12/Buffers/D3D12Buffers.h"
#include "Platform/DirectX12/Resources/D3D12ResourceManager.h"
#include "Platform/DirectX12/Textures/D3D12RenderTarget.h"

namespace Engine
{
	class D3D12RootSignature;

	struct WorldSettings;

	struct ObjectConstant;


	class D3D12RenderingApi : public RendererAPI
	{
	public:

		virtual ~D3D12RenderingApi() ;

		void Init(GraphicsContext* context, INT32 viewportWidth, INT32 viewportHeight) override;

		void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) override;

		void BindDepthPass(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems) override;

		void BindTerrainPass(PipelineStateObject* pso, RenderItem* terrain) override;

		void BindStaticGeoPass(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems) override;

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
			WorldSettings& settings,
			const MainCamera& camera,
			float deltaTime,
			float elapsedTime,
			bool wireframe
		) override;

		void PostRender() override;


		[[nodiscard]] GraphicsContext* GetGraphicsContext() const override { return Context; }

		[[nodiscard]] FrameBuffer* GetFrameBuffer() const override { return FrameBuffer.get(); };

		[[nodiscard]] MemoryManager* GetMemoryManager() const override{ return D3D12MemoryManager.get(); }

		[[nodiscard]] D3D12FrameResource* GetCurrentFrameResource() const { return CurrentFrameResource; }

		[[nodiscard]] RenderTarget* GetSceneTexture() const override { return RenderTarget.get(); }

	private:

		ComPtr<ID3DBlob> TerrainBuffer;

		// Buffer for uploading shader constants.
		ScopePointer<D3D12ResourceBuffer> UploadBuffer;

		// All of our frame resources.
		std::vector<ScopePointer<D3D12FrameResource>> FrameResources;
		UINT32 CurrentFrameResourceIndex = 0;
		// Keeps track of the current frame resource in flight.
		D3D12FrameResource* CurrentFrameResource = nullptr;


		// A pointer to the graphics context
		D3D12Context* Context = nullptr;

		// A unique pointer to the frame buffer
		ScopePointer<D3D12FrameBuffer> FrameBuffer = nullptr;
		// Custom render buffer
		ScopePointer<D3D12RenderTarget> RenderTarget = nullptr;

		ScopePointer<D3D12RenderTarget> ShadowMap = nullptr;
		
		// A unique pointer to the Api's memory allocator class.
		ScopePointer<D3D12MemoryManager> D3D12MemoryManager = nullptr;

		

	};

}

