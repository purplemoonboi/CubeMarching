#pragma once
#include <Framework/Renderer/Textures/Texture.h>

#include "Platform/DirectX12/Heap/D3D12HeapManager.h"


namespace Foundation::Graphics::D3D12
{

	class D3D12Texture : public Texture
	{
	public:

		D3D12Texture
		(
			const void* initData,
			UINT32 width,
			UINT32 height,
			ResourceFormat format = ResourceFormat::R8G8B8A8_UNORM
		);


		~D3D12Texture() override;

		void Destroy() override;

		void SetWidth(UINT32 width) override;
		void SetHeight(UINT32 height) override;
		void SetDepth(UINT16 depth) override;

		[[nodiscard]] virtual UINT64	GetTexture()			const override;

	private:


		ComPtr<ID3D12Resource> GpuResource;
		ComPtr<ID3D12Resource> UploadBuffer;
		BYTE* RawData = nullptr;

		D3D12DescriptorHandle pSRV;
		D3D12DescriptorHandle pUAV;

		UINT8 DirtyFlag{ 0 };

	};
}


