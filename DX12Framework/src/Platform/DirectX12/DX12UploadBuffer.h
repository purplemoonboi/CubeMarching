#pragma once
#include "DirectX12.h"

#include "DX12BufferUtils.h"

namespace DX12Framework
{



	// Using namespace
	using Microsoft::WRL::ComPtr;

	template <typename T>
	class DX12UploadBuffer
	{
	public:

		DX12UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer)
			:
			IsConstantBuffer(isConstantBuffer)
		{

			ElementByteSize = sizeof(T);

			if(isConstantBuffer)
			{
				ElementByteSize = DX12BufferUtils::CalculateConstantBufferByteSize(sizeof(T));
			}

			THROW_ON_FAILURE
			(
				device->CreateCommittedResource
				(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Buffer(ElementByteSize * elementCount),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&UploadBuffer)
				)
			);

			THROW_ON_FAILURE(UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(MappedData)));

		}

		DX12UploadBuffer(const DX12UploadBuffer& rhs) = delete;
		DX12UploadBuffer& operator=(const DX12UploadBuffer& rhs) = delete;
		virtual ~DX12UploadBuffer()
		{
			if(UploadBuffer != nullptr)
			{
				UploadBuffer->Unmap(0, nullptr);
			}

			MappedData = nullptr;
		}

		ID3D12Resource* Resource() const { return UploadBuffer.Get(); }

		void CopyData(INT32 elementIndex, const T& data)
		{
			memcpy(&MappedData[elementIndex * ElementByteSize], &data, sizeof(T));
		}

	private:
		ComPtr<ID3D12Resource> UploadBuffer;
		BYTE* MappedData = nullptr;
		UINT ElementByteSize = 0U;
		bool IsConstantBuffer = false;

	};
}

