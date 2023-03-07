#pragma once
#include "../DirectX12.h"
#include "Framework/Core/Log/Log.h"

namespace Engine
{

	using Microsoft::WRL::ComPtr;

	class D3D12BufferUtils
	{
	public:

		static void Init
		(
			ID3D12Device* device,
			ID3D12GraphicsCommandList* graphicsCmdList
		);

		// @brief Creates a default upload buffer.
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
			const void* initData,
			DXGI_FORMAT format,
			ComPtr<ID3D12Resource>& uploadBuffer
		);

		static void CopyTexture(ID3D12Resource* src, ID3D12Resource* dst);

		static void CopyTextureGPU(ID3D12Resource* src, ID3D12Resource* dst);

		static ComPtr<ID3D12Resource> CreateCounterResource(bool allowShaderAtomics, bool allowWrite = false);

		static ComPtr<ID3D12Resource> CreateReadBackBuffer(UINT32 bufferWidth);

		static ComPtr<ID3D12Resource> CreateStructuredBuffer(UINT32 bufferWidth, bool allowWrite = false, bool allowAtomics = false);

		static void CreateUploadBuffer(ComPtr<ID3D12Resource>& resource, UINT32 bufferWidth);

		static UINT CalculateConstantBufferByteSize(UINT byteSize);


	private:
		static ID3D12Device* Device;
		static ID3D12GraphicsCommandList* GraphicsCmdList;
	};
}
