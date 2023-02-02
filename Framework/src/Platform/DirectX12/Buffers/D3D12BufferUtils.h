#pragma once
#include "../DirectX12.h"

namespace Engine
{

	using Microsoft::WRL::ComPtr;

	class D3D12BufferUtils
	{
	public:

		// @brief Creates a default upload buffer.
		// @param[in] A pointer to the application's device.
		// @param[in] A pointer to the graphics command list.
		// @param[in] A pointer to the initial vertex data.
		// @param[in] The WorldSize in bytes
		// @param[in] A reference to the upload buffer.
		static ComPtr<ID3D12Resource> Create_Vertex_UploadAndDefaultBuffers
		(
			ID3D12Device* device,
			ID3D12GraphicsCommandList* graphicsCmdList,
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
		static ComPtr<ID3D12Resource> Create_Texture3D_UploadAndDefaultBuffers
		(
			ID3D12Device* device,
			ID3D12GraphicsCommandList* graphicsCmdList,
			UINT32 width,
			UINT32 height,
			UINT16 depth,
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
		static ComPtr<ID3D12Resource> Create_Texture2D_UploadAndDefaultBuffers
		(
			ID3D12Device* device,
			ID3D12GraphicsCommandList* graphicsCmdList,
			UINT32 width,
			UINT32 height,
			const void* initData,
			DXGI_FORMAT format,
			ComPtr<ID3D12Resource>& uploadBuffer
		);

		static UINT CalculateConstantBufferByteSize(UINT byteSize);

	};
}
