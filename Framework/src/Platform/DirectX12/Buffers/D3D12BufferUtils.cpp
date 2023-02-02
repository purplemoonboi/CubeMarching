#include "D3D12BufferUtils.h"

#include "Framework/Core/Log/Log.h"


namespace Engine
{
	// Upload buffer methods
	ComPtr<ID3D12Resource> D3D12BufferUtils::Create_Vertex_UploadAndDefaultBuffers
	(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* graphicsCmdList,
		const void* initData,
		UINT64 byteSize,
		ComPtr<ID3D12Resource>& uploadBuffer
	)
	{
		ComPtr<ID3D12Resource> defaultBuffer;


		//Create the committed resource
		THROW_ON_FAILURE(device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(defaultBuffer.GetAddressOf())
		));


		THROW_ON_FAILURE(device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		));

		// Give a desc of the data we want to copy
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = initData;
		subResourceData.RowPitch = byteSize;
		subResourceData.SlicePitch = subResourceData.RowPitch;

		// Schedule to copy the data to the default buffer resource.
		// Make instruction to copy CPU buffer into intermediate upload heap
		// buffer.
		graphicsCmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				defaultBuffer.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_COPY_DEST
			)
		);

		// Copy the data into the upload heap
		UpdateSubresources<1>
			(
				graphicsCmdList,
				defaultBuffer.Get(),
				uploadBuffer.Get(),
				0,
				0,
				1,
				&subResourceData
				);

		// Add the instruction to transition back to read 
		graphicsCmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				defaultBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_GENERIC_READ
			)
		);

		// IMPORTANT: The upload buffer must be kept in scope after the above function calls. This is
		//			  because the cmd list has NOT executed the copy.
		//
		//			  The buffer can be released after the caller knows the copy has been made.
		return defaultBuffer;
	}

	ComPtr<ID3D12Resource> D3D12BufferUtils::Create_Texture3D_UploadAndDefaultBuffers
	(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* graphicsCmdList,
		UINT32 width,
		UINT32 height,
		UINT16 depth,
		const void* initData,
		DXGI_FORMAT format,
		ComPtr<ID3D12Resource>& uploadBuffer
	)
	{
		ComPtr<ID3D12Resource> defaultBuffer;
		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		texDesc.Alignment = 0;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.DepthOrArraySize = depth;
		texDesc.MipLevels = 1;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		//Create the committed resource
		const HRESULT defaultResult = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&defaultBuffer)
		);
		THROW_ON_FAILURE(defaultResult);

		const UINT64 uploadBufferSize = sizeof(float) * width * height * depth;

		HRESULT uploadResult = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		);
		THROW_ON_FAILURE(uploadResult);

		// Give a desc of the data we want to copy
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = initData;
		subResourceData.RowPitch = sizeof(float) * width;
		subResourceData.SlicePitch = subResourceData.RowPitch * height * depth;
		

		// Schedule to copy the data to the default buffer resource.
		// Make instruction to copy CPU buffer into intermediate upload heap
		// buffer.
		graphicsCmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				defaultBuffer.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_COPY_DEST
			)
		);

		// Copy the data into the upload heap
		UpdateSubresources<1>
			(
				graphicsCmdList,
				defaultBuffer.Get(),
				uploadBuffer.Get(),
				0,
				0,
				1,
				&subResourceData
				);

		// Add the instruction to transition back to read 
		graphicsCmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				defaultBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_COMMON
			)
		);

		// IMPORTANT: The upload buffer must be kept in scope after the above function calls. This is
		//			  because the cmd list has NOT executed the copy.
		//
		//			  The buffer can be released after the caller knows the copy has been made.
		return defaultBuffer;
	}

	ComPtr<ID3D12Resource> D3D12BufferUtils::Create_Texture2D_UploadAndDefaultBuffers
	(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* graphicsCmdList, 
		UINT32 width,
		UINT32 height,
		const void* initData,
		DXGI_FORMAT format,
		ComPtr<ID3D12Resource>& uploadBuffer
	)
	{
		ComPtr<ID3D12Resource> defaultBuffer;
		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		texDesc.Alignment = 0;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		//Create the committed resource
		THROW_ON_FAILURE(device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(defaultBuffer.GetAddressOf())
		));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(defaultBuffer.Get(),
			0, texDesc.DepthOrArraySize * texDesc.MipLevels);
		THROW_ON_FAILURE(device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		));

		// Give a desc of the data we want to copy
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = initData;
		subResourceData.RowPitch = 16 * width;
		subResourceData.SlicePitch = subResourceData.RowPitch;

		// Schedule to copy the data to the default buffer resource.
		// Make instruction to copy CPU buffer into intermediate upload heap
		// buffer.
		graphicsCmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				defaultBuffer.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_COPY_DEST
			)
		);

		// Copy the data into the upload heap
		UpdateSubresources<1>
			(
				graphicsCmdList,
				defaultBuffer.Get(),
				uploadBuffer.Get(),
				0,
				0,
				1,
				&subResourceData
				);

		// Add the instruction to transition back to read 
		graphicsCmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				defaultBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_GENERIC_READ
			)
		);

		// IMPORTANT: The upload buffer must be kept in scope after the above function calls. This is
		//			  because the cmd list has NOT executed the copy.
		//
		//			  The buffer can be released after the caller knows the copy has been made.
		return defaultBuffer;
	}

	UINT D3D12BufferUtils::CalculateConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}
}
