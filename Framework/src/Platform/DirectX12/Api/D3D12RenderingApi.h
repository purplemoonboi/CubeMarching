#pragma once
#include "Framework/Renderer/Api/RendererAPI.h"
#include "Platform/DirectX12/Constants/D3D12GlobalConstants.h"


namespace Foundation::Graphics::D3D12
{
	class D3D12Context;
	class D3D12FrameResource;
	class D3D12ResourceBuffer;
	class D3D12FrameBuffer;
	class D3D12RenderTarget;
	class D3D12DescriptorHeap;
	class D3D12PipelineStateObject;
	class D3D12RootSignature;


	class D3D12RenderingPipelines
	{
	public:
		D3D12RenderingPipelines() = default;
		DISABLE_COPY_AND_MOVE(D3D12RenderingPipelines);

		void Insert(const std::string& name, D3D12PipelineStateObject* pso);
		void InsertStateObject(const std::string& name, ID3D12StateObject* pso);

		void Remove(const std::string& name);
		void RemoveStateObject(const std::string& name);

	public:/*...Getters...*/

		[[nodiscard]] D3D12PipelineStateObject* Get(const std::string& name) const { return pPipelineStates.at(name); }
		[[nodiscard]] ID3D12StateObject* GetStateObject(const std::string& name) const { return pPipelineObjects.at(name); }

	private:
		std::unordered_map<std::string, D3D12PipelineStateObject*> pPipelineStates;

		std::unordered_map<std::string, ID3D12StateObject*> pPipelineObjects;
	};


	class D3D12RenderingApi : public RendererAPI
	{
	public:
		D3D12RenderingApi();

		virtual ~D3D12RenderingApi();

		void Init(GraphicsContext* context, INT32 viewportWidth, INT32 viewportHeight) override;
		void Clean() override;

		void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) override;
		[[nodiscard]] FrameBufferSpecifications GetViewportSpecifications() const override { return BufferSpecs; }

		void PreInit() override;
		void PostInit() override;

		void BindPasses() override;

		void OnPreBeginRender
		(
			MainCamera* camera,
			AppTimeManager* time,
			const std::vector<RenderItem*>& items, 
			const std::vector<Material*>& materials,
			bool wireframe
		) override;

		void OnBeginRender() override;
		void OnEndRender() override;

		void Flush() override;

		void DrawSceneStaticGeometry(PipelineStateObject* pso, const std::vector<RenderItem*>& renderItems) override;
		void DrawIndexed(const RefPointer<VertexArray>& vertexArray, INT32 indexCount = 0) override {}
		void DrawIndexed(const ScopePointer<MeshGeometry>& geometry, INT32 indexCount = 0) override {}

		[[nodiscard]] GraphicsContext*	  GetGraphicsContext()					const override;				
		[[nodiscard]] FrameBuffer*		  GetFrameBuffer()						const override;	
;
		[[nodiscard]] const RenderTarget* GetSceneAlbedoTexture()				const override; 
		[[nodiscard]] const RenderTarget* GetSceneNormalTexture()				const override; 
		[[nodiscard]] const RenderTarget* GetSceneAmbientOcclusionTexture()		const override; 
		[[nodiscard]] const RenderTarget* GetSceneDepthTexture()				const override; 

		[[nodiscard]] D3D12FrameResource* GetFrame() const { return Frames[CurrentFrame].get(); }
	private:
		// A pointer to the graphics context
		D3D12Context* Context{ nullptr };

		FrameBufferSpecifications BufferSpecs;

		D3D12RenderingPipelines Pipeline;

	

		ScopePointer<D3D12ResourceBuffer>		UploadBuffer		{ nullptr };
		ScopePointer<D3D12FrameBuffer>			FrameBuffer			{ nullptr };
		ScopePointer<D3D12RenderingPipelines>	RenderingPipelines	{ nullptr };

		std::array<ScopePointer<D3D12FrameResource>, FRAMES_IN_FLIGHT> Frames{ nullptr };

		std::array<ScopePointer<D3D12RenderTarget>, GBUFFER_SIZE> RenderTargets;

		ComPtr<ID3D12RootSignature> pRootSignature;

		UINT32 CurrentFrame{ 0 };


	};

}

