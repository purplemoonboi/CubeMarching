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
		D3D12FrameBuffer(const FrameBufferSpecifications& fBufferSpecs, ResourceFormat format);
		DISABLE_COPY_AND_MOVE(D3D12FrameBuffer);

		~D3D12FrameBuffer() override;


		void Bind() override;
		void UnBind() override;
		void OnResizeFrameBuffer(FrameBufferSpecifications& specifications) override;

		void SetBackBufferIndex(INT32 value) { BackBufferIndex = value; }

	public:/*...Getters...*/

		[[nodiscard]] const D3D12_VIEWPORT& GetViewport() const { return ScreenViewport; }
		[[nodiscard]] const D3D12_RECT& GetScissorsRect() const { return ScissorRect; }
		[[nodiscard]] ID3D12Resource* CurrentBackBuffer() const;
		[[nodiscard]] UINT64 GetFrameBuffer() const override;
		[[nodiscard]] INT32 GetBackBufferIndex() const { return BackBufferIndex; }
		
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferViewCpu() const;
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilViewCpu() const;
	
		[[nodiscard]] DXGI_FORMAT GetBackBufferFormat() const { return BackBufferFormat; }
		[[nodiscard]] DXGI_FORMAT GetDepthStencilFormat() const { return DepthStencilFormat; }


	private:
		ComPtr<ID3D12Resource>				TargetBuffer[FRAMES_IN_FLIGHT]{nullptr};
		ComPtr<ID3D12Resource>				DepthStencilBuffer{nullptr};

		D3D12DescriptorHandle pRTV[FRAMES_IN_FLIGHT] = {{}};
		D3D12DescriptorHandle pDSV = {};

		D3D12_VIEWPORT ScreenViewport;
		D3D12_RECT ScissorRect;

		DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		INT32 BackBufferIndex;

	};

}
