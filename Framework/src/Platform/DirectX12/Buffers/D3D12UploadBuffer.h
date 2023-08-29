#pragma once
#include "Framework/Core/Core.h"
#include "Platform/DirectX12/DirectX12.h"

namespace Foundation::Graphics::D3D12
{
	// Using namespace
	using Microsoft::WRL::ComPtr;

	class D3D12UploadBuffer
	{
	public:

		D3D12UploadBuffer(UINT32 elementCount, UINT64 size, bool isConstantBuffer);
		DISABLE_COPY_AND_MOVE(D3D12UploadBuffer);

		virtual ~D3D12UploadBuffer();

		ID3D12Resource* Resource() const { return UploadBuffer.Get(); }

		void CopyData(INT32 elementIndex, const BYTE* data)
		{
			memcpy(&MappedData[elementIndex * ElementByteSize], &data, ElementByteSize);
		}

		void Destroy()
		{
			
		}

	private:
		ComPtr<ID3D12Resource> UploadBuffer;
		BYTE* MappedData;
		UINT ElementByteSize;
		UINT32 ElementCount;
		bool IsConstantBuffer;

	};
}

