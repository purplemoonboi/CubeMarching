#pragma once
#include "../DirectX12.h"
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Foundation::Graphics::D3D12
{
	// Using namespace
	using Microsoft::WRL::ComPtr;

	class D3D12UploadBuffer
	{
	public:

		D3D12UploadBuffer(D3D12Context* graphicsContext, UINT32 elementCount, UINT64 size, bool isConstantBuffer);
		DISABLE_COPY_AND_MOVE(D3D12UploadBuffer);

		virtual ~D3D12UploadBuffer();

		ID3D12Resource* Resource() const { return UploadBuffer.Get(); }

		void CopyData(INT32 elementIndex, const BYTE* data)
		{
			memcpy(&MappedData[elementIndex * ElementByteSize], &data, ElementByteSize);
		}

		void Destroy()
		{
			UploadBuffer.Reset();
		}

	private:
		ComPtr<ID3D12Resource> UploadBuffer;
		BYTE* MappedData;
		UINT ElementByteSize;
		UINT32 ElementCount;
		bool IsConstantBuffer;

	};
}

