#pragma once
#include "Platform/DirectX12/DirectX12.h"

namespace Foundation::Graphics::D3D12
{

	class D3D12BufferFactory
	{
	public:
		D3D12BufferFactory() = default;

		static void Init
		(
			ID3D12GraphicsCommandList4* graphicsCmdList
		);

		// @brief Creates a default resource buffer on the GPU.
		// @param[in] A pointer to the application's device.
		// @param[in] A pointer to the graphics command list.
		// @param[in] A pointer to the initial vertex data.
		// @param[in] The WorldSize in bytes
		// @param[in] A reference to the upload buffer.
		static ComPtr<ID3D12Resource> CreateDefaultBuffer
		(
			const void* initData,
			UINT64 byteSize,
			ComPtr<ID3D12Resource>& uploadBuffer
		);

		// @brief Creates a default upload buffer.
		// @param[in] A pointer to the application's device.
		// @param[in] A pointer to the graphics command list.
		// @param[in] A pointer to the initial vertex data.
		// @param[in] The WorldSize in bytes
		// @param[in] A reference to the upload buffer.
		static ComPtr<ID3D12Resource> CreateTexture3D
		(
			UINT32 width,
			UINT32 height,
			UINT16 depth,
			UINT16 mipLevels,
			const void* initData,
			DXGI_FORMAT format,
			ComPtr<ID3D12Resource>& uploadBuffer
		);

		// @brief Creates a default upload buffer.
		// @param[in] A pointer to the application's device.
		// @param[in] A pointer to the graphics command list.
		// @param[in] A pointer to the initial vertex data.
		// @param[in] The WorldSize in bytes
		// @param[in] A reference to the upload buffer.
		static ComPtr<ID3D12Resource> CreateTexture2D
		(
			UINT32 width,
			UINT32 height,
			UINT16 mipLevels,
			const void* pData,
			DXGI_FORMAT format,
			ComPtr<ID3D12Resource>& uploadBuffer
		);

		static ComPtr<ID3D12Resource> CreateRenderTexture(INT32 width, INT32 height, DXGI_FORMAT format);
		static ComPtr<ID3D12Resource> CreateCounterResource(bool allowShaderAtomics, bool allowWrite = false);
		static ComPtr<ID3D12Resource> CreateReadBackBuffer(UINT32 bufferWidth);
		static ComPtr<ID3D12Resource> CreateStructuredBuffer(UINT32 bufferWidth, bool allowWrite = false, bool allowAtomics = false);
		static ComPtr<ID3D12Resource> CreateStructuredBuffer(UINT32 bufferWidth, ID3D12Resource* uploadBuffer = nullptr, const void* data = nullptr, bool allowWrite = false, bool allowAtomics = false);

		static void CreateUploadBuffer(ComPtr<ID3D12Resource>& resource, UINT32 bufferWidth);

		static UINT CalculateBufferByteSize(UINT byteSize);


	private:
		static ID3D12GraphicsCommandList4* pGCL;
	};
}
