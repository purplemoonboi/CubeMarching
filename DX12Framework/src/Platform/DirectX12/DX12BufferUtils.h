#pragma once
#include "DirectX12.h"

namespace DX12Framework
{

	using Microsoft::WRL::ComPtr;

	class DX12BufferUtils
	{
	public:

		// @brief Creates a default upload buffer.
		// @param[in] A pointer to the application's device.
		// @param[in] A pointer to the graphics command list.
		// @param[in] A pointer to the initial vertex data.
		// @param[in] The size in bytes
		// @param[in] A reference to the upload buffer.
		static ComPtr<ID3D12Resource> CreateDefaultBuffer
		(
			ID3D12Device* device,
			ID3D12GraphicsCommandList* graphicsCmdList,
			const void* initData,
			UINT64 byteSize,
			ComPtr<ID3D12Resource>& uploadBuffer
		);


		static UINT CalculateConstantBufferByteSize(UINT byteSize);

	};
}
