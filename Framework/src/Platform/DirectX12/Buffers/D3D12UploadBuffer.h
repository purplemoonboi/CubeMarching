#pragma once
#include "../DirectX12.h"
#include "D3D12BufferUtils.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Engine
{



	// Using namespace
	using Microsoft::WRL::ComPtr;

	template <typename T>
	class D3D12UploadBuffer
	{
	public:

		D3D12UploadBuffer(D3D12Context* graphicsContext, UINT elementCount, bool isConstantBuffer)
			:
			IsConstantBuffer(isConstantBuffer)
		{


			ElementByteSize = sizeof(T);

			if(isConstantBuffer)
			{
				ElementByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(sizeof(T));
			}

			THROW_ON_FAILURE
			(
				graphicsContext->Device->CreateCommittedResource
				(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Buffer(ElementByteSize * elementCount),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&UploadBuffer)
				)
			);

			THROW_ON_FAILURE(UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&MappedData)));

		}

		D3D12UploadBuffer(const D3D12UploadBuffer& rhs) = delete;
		D3D12UploadBuffer& operator=(const D3D12UploadBuffer& rhs) = delete;

		void Bind(UINT index, const D3D12_RANGE* range)
		{
			UploadBuffer->Map(index, range, reinterpret_cast<void**>(&MappedData));
		}

		void UnBind(UINT index, const D3D12_RANGE* range = nullptr) const
		{
			UploadBuffer->Unmap(0, nullptr);
		}

		virtual ~D3D12UploadBuffer()
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

