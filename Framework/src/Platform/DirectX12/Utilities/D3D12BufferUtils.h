#pragma once
#include "../DirectX12.h"

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

		template<typename T> 
		static ComPtr<ID3D12Resource> CreateComputeBuffer()
		{
			ComPtr<ID3D12Resource> buffer = nullptr;
			constexpr auto bufferWidth = (NumberOfBufferElements * sizeof(T));
			const HRESULT result = ComputeContext->Context->Device->CreateCommittedResource
			(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS,
				&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				nullptr,
				IID_PPV_ARGS(&OutputBuffer)
			);
			THROW_ON_FAILURE(result);

			return buffer;
		}

		// @brief Creates a default upload buffer.
		// @param[in] A pointer to the application's device.
		// @param[in] A pointer to the graphics command list.
		// @param[in] A pointer to the initial vertex data.
		// @param[in] The WorldSize in bytes
		// @param[in] A reference to the upload buffer.
		static ComPtr<ID3D12Resource> CreateVertexBuffer
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
			const void* initData,
			DXGI_FORMAT format,
			ComPtr<ID3D12Resource>& uploadBuffer
		);

		static ComPtr<ID3D12Resource> CreateCounterResource(UINT32 counter);

		template<typename T>
		static void CreateUploadBuffer(ComPtr<ID3D12Resource> resource, UINT32 bufferCapacity)
		{
			constexpr auto bufferWidth = sizeof(T) * bufferCapacity 
			const HRESULT result = Device->CreateCommittedResource
			(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(resource.GetAddressOf())
			);
			THROW_ON_FAILURE(result);

		}

		template<typename T>
		static ComPtr<ID3D12Resource> CreateReadBackBuffer(UINT32 bufferCapacity)
		{
			ComPtr<ID3D12Resource> readbackResource = nullptr;

			constexpr auto bufferWidth = (bufferCapacity * sizeof(T));

			const HRESULT readBackResult = Device->CreateCommittedResource
			(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth),
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&readBackResource)
			);

			THROW_ON_FAILURE(readBackResult);

			return readBackResult;
		}

		static UINT CalculateConstantBufferByteSize(UINT byteSize);
	private:
		static ID3D12Device* Device;
		static ID3D12GraphicsCommandList* GraphicsCmdList;
	};
}
