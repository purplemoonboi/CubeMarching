#include "D3D12UploadBuffer.h"
#include "Platform/DirectX12/D3D12/D3D12.h"
#include "Platform/DirectX12/Utilities/D3D12BufferUtils.h"

namespace Foundation::Graphics::D3D12
{

	D3D12UploadBuffer::D3D12UploadBuffer(UINT32 elementCount, UINT64 size, bool isConstantBuffer)
		:
			IsConstantBuffer(isConstantBuffer)
		,	ElementSize(size)
		,	ElementCount(elementCount)
		,	MappedData(nullptr)
	{
		auto pDevice = gD3D12Context->GetDevice();

		if (isConstantBuffer)
		{
			ElementByteSize = D3D12BufferUtils::CalculateConstantBufferByteSize(size);
		}

		const HRESULT uploadResult = pDevice->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(ElementByteSize * elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&UploadBuffer)
		);
		THROW_ON_FAILURE(uploadResult);

		THROW_ON_FAILURE(UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&MappedData)));

	}

	void D3D12UploadBuffer::CopyData(INT32 elementIndex, const BYTE* data)
	{
		memcpy(&MappedData[elementIndex * ElementByteSize], &data, ElementByteSize);
	}

}