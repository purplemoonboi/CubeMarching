#pragma once
#include "Framework/Core/Core.h"
#include "Platform/DirectX12/_D3D12ConstantExpressions/D3D12ConstantExpressions.h"

#include "Framework/Renderer/Buffers/FrameBuffer.h"

#include "../DirectX12.h"


namespace Engine
{
	using Microsoft::WRL::ComPtr;


	class D3D12Context;

	class D3D12FrameBuffer : public FrameBuffer
	{
	public:

		D3D12FrameBuffer(const D3D12FrameBuffer& other);
		D3D12FrameBuffer(const FrameBufferSpecifications& fBufferSpecs);
		~D3D12FrameBuffer() override;

		void Init(GraphicsContext* context) override;

		void Bind() override;
		void UnBind() override;
		void RebuildFrameBuffer(INT32 width, INT32 height) override;

		void SetViewportDimensions(INT32 width, INT32 height);
		void SetBackBufferIndex(INT32 value) { BackBufferIndex = value; }

		[[nodiscard]] const FrameBufferSpecifications& GetSpecifications() const override { return FrameBufferSpecs; }
		[[nodiscard]] const D3D12_VIEWPORT& GetViewport() const { return ScreenViewport; }
		[[nodiscard]] const D3D12_RECT& GetScissorsRect() const { return ScissorRect; }
		[[nodiscard]] ID3D12Resource* CurrentBackBuffer() const;
		[[nodiscard]] void* GetFrameBuffer() const override;
		[[nodiscard]] INT32 GetBackBufferIndex() const { return BackBufferIndex; }
		[[nodiscard]] UINT32 GetRtvDescSize() const { return RtvDescriptorSize; }
		[[nodiscard]] UINT32 GetDsvDescSize() const { return DsvDescriptorSize; }
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferViewCpu() const;
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilViewCpu() const;
		[[nodiscard]] INT32 GetWidth() const override;
		[[nodiscard]] INT32 GetHeight() const override;
		[[nodiscard]] DXGI_FORMAT GetBackBufferFormat() const { return BackBufferFormat; }
		[[nodiscard]] DXGI_FORMAT GetDepthStencilFormat() const { return DepthStencilFormat; }

		// @brief - Represents the size of the RTV descriptor heap.
		UINT32 RtvDescriptorSize;
		// @brief - Represents the size of the DSV descriptor heap.
		UINT32 DsvDescriptorSize;

	private:
		/**
		 * @brief Main render target buffer.
		 */
		ComPtr<ID3D12Resource>				SwapChainBuffer[SWAP_CHAIN_BUFFER_COUNT];

		// @brief Heap descriptor for resources
		ComPtr<ID3D12DescriptorHeap> RtvHeap;
		// @brief Heap descriptor for depth-stencil resource
		ComPtr<ID3D12DescriptorHeap> DsvHeap;
		// @brief - A structure describing the buffer which we render to.
		D3D12_VIEWPORT ScreenViewport;
		// @brief - Used to specify an area of the buffer we would like to render to.
		//			Therefore culls pixels not included in the scissor rect.
		D3D12_RECT ScissorRect;

		// @brief Defines the format of the back buffer.
		DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		// @brief Defines the format of the stencil.
		DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	private:
		INT32 BackBufferIndex;
		FrameBufferSpecifications FrameBufferSpecs;

		D3D12Context* Context = nullptr;
	};

}
