#pragma once
#include "../DirectX12.h"
#include "Platform/DirectX12/Utilities/D3D12BufferFactory.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Foundation::Graphics::D3D12
{
	// Using namespace
	using Microsoft::WRL::ComPtr;

	template <typename T>
	class D3D12UploadBuffer
	{
	public:

		D3D12UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer)
			:
			IsConstantBuffer(isConstantBuffer)
		{


			ElementByteSize = sizeof(T);

			if(isConstantBuffer)
			{
				ElementByteSize = D3D12BufferFactory::CalculateBufferByteSize(sizeof(T));
			}

			const HRESULT uploadResult = device->CreateCommittedResource
			(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(ElementByteSize * elementCount),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&pUpload)
			);
			THROW_ON_FAILURE(uploadResult);

			THROW_ON_FAILURE(pUpload->Map(0, nullptr, reinterpret_cast<void**>(&MappedData)));

		}

		//DISABLE_COPY(D3D12UploadBuffer);

		void Bind(UINT index, const D3D12_RANGE* range)
		{
			pUpload->Map(index, range, reinterpret_cast<void**>(&MappedData));
		}

		void UnBind(UINT index, const D3D12_RANGE* range = nullptr) const
		{
			pUpload->Unmap(0, nullptr);
		}

		virtual ~D3D12UploadBuffer()
		{
			if(pUpload != nullptr)
			{
				pUpload->Unmap(0, nullptr);
			}

			MappedData = nullptr;
		}

		void CopyData(INT32 elementIndex, const T& data)
		{
			memcpy(&MappedData[elementIndex * ElementByteSize], &data, sizeof(T));
		}

		void Destroy()
		{
			pUpload.Reset();
		}

		void Create(const D3D12Context* context, UINT elementCount, bool isStatic)
		{
			ElementByteSize = sizeof(T);

			if (isStatic)
			{
				ElementByteSize = D3D12BufferFactory::CalculateBufferByteSize(sizeof(T));
			}

			THROW_ON_FAILURE(context->pDevice->CreateCommittedResource
			(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(ElementByteSize * elementCount),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&pUpload)
			));

			THROW_ON_FAILURE(pUpload->Map(0, nullptr, reinterpret_cast<void**>(&MappedData)));
		}


	public:/*...Getters...*/
		[[nodiscard]]
		ID3D12Resource* Resource() const { return pUpload.Get(); }


	private:
		ComPtr<ID3D12Resource> pUpload;
		BYTE* MappedData = nullptr;
		UINT ElementByteSize = 0U;
		bool IsConstantBuffer = false;

	};
}

