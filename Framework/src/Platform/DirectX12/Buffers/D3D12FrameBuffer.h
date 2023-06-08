#pragma once
#include "Framework/Core/Core.h"

#include "Framework/Renderer/Buffers/FrameBuffer.h"

#include "../DirectX12.h"
#include "Platform/DirectX12/Textures/D3D12Texture.h"


namespace Foundation::Graphics::D3D12
{
	using Microsoft::WRL::ComPtr;

	class D3D12Texture;
	class D3D12Context;

	class D3D12FrameBuffer : public FrameBuffer
	{
	public:
		D3D12FrameBuffer(const FrameBufferSpecifications& fBufferSpecs);
		DISABLE_COPY_AND_MOVE(D3D12FrameBuffer);

		~D3D12FrameBuffer() override;

		void Init(GraphicsContext* context, FrameBufferSpecifications& fbs) override;

		void Bind(void* args) override;
		void UnBind(void* args) override;
		void OnResizeFrameBuffer(FrameBufferSpecifications& specifications) override;


		void SetBufferSpecifications(FrameBufferSpecifications& fbSpecs) override;

		void SetBackBufferIndex(INT32 value) { BackBufferIndex = value; }

	public:/*...Getters...*/

		[[nodiscard]] const FrameBufferSpecifications& GetSpecifications() const override { return FrameBufferSpecs; }
		[[nodiscard]] const D3D12_VIEWPORT& GetViewport() const { return ScreenViewport; }
		[[nodiscard]] const D3D12_RECT& GetScissorsRect() const { return ScissorRect; }
		[[nodiscard]] ID3D12Resource* CurrentBackBuffer() const;
		[[nodiscard]] UINT64 GetFrameBuffer() const override;
		[[nodiscard]] INT32 GetBackBufferIndex() const { return BackBufferIndex; }
		
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferViewCpu() const;
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilViewCpu() const;
		[[nodiscard]] INT32 GetWidth() const override;
		[[nodiscard]] INT32 GetHeight() const override;
		[[nodiscard]] DXGI_FORMAT GetBackBufferFormat() const { return BackBufferFormat; }
		[[nodiscard]] DXGI_FORMAT GetDepthStencilFormat() const { return DepthStencilFormat; }


	private:

		/**
		 * @brief Main render target buffer.
		 */
		ComPtr<ID3D12Resource>				SwapChainBuffer[SWAP_CHAIN_BUFFER_COUNT]{nullptr};
		ComPtr<ID3D12Resource>				DepthStencilBuffer{nullptr};

		CD3DX12_CPU_DESCRIPTOR_HANDLE RenderTargetHandles[2] = {};
		CD3DX12_CPU_DESCRIPTOR_HANDLE DepthStencilHandle = {};

		D3D12_VIEWPORT ScreenViewport;
		D3D12_RECT ScissorRect;

		DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	private:
		INT32 BackBufferIndex;
		FrameBufferSpecifications FrameBufferSpecs;

		D3D12Context* Context = nullptr;
	};

}
