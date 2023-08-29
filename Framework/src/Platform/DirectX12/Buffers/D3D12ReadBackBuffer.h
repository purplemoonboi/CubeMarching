#pragma once

#include "Platform/DirectX12/Resources/D3D12FrameResource.h"


namespace Engine
{
	using Microsoft::WRL::ComPtr;

	class D3D12ReadBackBuffer
	{
	public:

		template<typename Type>
		HRESULT Create(UINT64 bufferSize, ID3D12Device* device)
		{
			constexpr auto bufferWidth = (bufferSize * sizeof(Type));

			/*create a readback buffer*/
			const HRESULT readbackResult = device->CreateCommittedResource
			(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth),
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&Buffer)
			);
			THROW_ON_FAILURE(readbackResult);

			return readbackResult;
		}

		ComPtr<ID3D12Resource> Buffer = nullptr;
	};
}
