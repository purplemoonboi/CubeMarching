#include "D3D12BufferFactory.h"

#include <Framework/Core/Log/Log.h>

#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Foundation::Graphics::D3D12
{

	ID3D12GraphicsCommandList4* D3D12BufferFactory::pGCL = nullptr;

	void D3D12BufferFactory::Init(ID3D12GraphicsCommandList4* graphicsCmdList)
	{
		pGCL = graphicsCmdList;
	}

	// Upload buffer methods
	ComPtr<ID3D12Resource> D3D12BufferFactory::CreateDefaultBuffer
	(
		const void* initData,
		UINT64 byteSize,
		ComPtr<ID3D12Resource>& uploadBuffer
	)
	{
		const auto device = gD3D12Context->GetDevice();
		CORE_ASSERT(device, "Invalid device!");

		ComPtr<ID3D12Resource> defaultBuffer;

		const HRESULT deviceCr = device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceCr);

		//Create the committed resource
		const HRESULT vertexResult = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(defaultBuffer.GetAddressOf())
		);
		THROW_ON_FAILURE(vertexResult);

		const HRESULT uploadResult = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		);
		THROW_ON_FAILURE(uploadResult);

		if(initData != nullptr)
		{
			// Give a desc of the data we want to copy
			D3D12_SUBRESOURCE_DATA subResourceData = {};
			subResourceData.pData = initData;
			subResourceData.RowPitch = byteSize;
			subResourceData.SlicePitch = subResourceData.RowPitch;

			// Schedule to copy the data to the default buffer resource.
			// Make instruction to copy CPU buffer into intermediate upload heap
			// buffer.
			pGCL->ResourceBarrier
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
					pGCL,
					defaultBuffer.Get(),
					uploadBuffer.Get(),
					0,
					0,
					1,
					&subResourceData
					);

			// Add the instruction to transition back to read 
			pGCL->ResourceBarrier
			(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition
				(
					defaultBuffer.Get(),
					D3D12_RESOURCE_STATE_COPY_DEST,
					D3D12_RESOURCE_STATE_GENERIC_READ
				)
			);

		}

		return defaultBuffer;
	}

	ComPtr<ID3D12Resource> D3D12BufferFactory::CreateTexture3D
	(
		UINT32 width,
		UINT32 height,
		UINT16 depth,
		UINT16 mipLevels,
		const void* initData,
		DXGI_FORMAT format,
		ComPtr<ID3D12Resource>& uploadBuffer
	)
	{
		const auto device = gD3D12Context->GetDevice();
		CORE_ASSERT(device, "Invalid device!");

		ComPtr<ID3D12Resource> defaultBuffer;
		D3D12_RESOURCE_DESC texDesc{};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		texDesc.Alignment = 0;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.DepthOrArraySize = depth;
		texDesc.MipLevels = mipLevels;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

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

		UINT32 numOfResources = 1;

		if (texDesc.DepthOrArraySize > 6 || texDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
		{
			numOfResources = 1;
		}
		else
		{
			numOfResources = texDesc.DepthOrArraySize * texDesc.MipLevels;
		}

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(defaultBuffer.Get(),
			0, numOfResources);

		const HRESULT uploadResult = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		);
		THROW_ON_FAILURE(uploadResult);

		//...if there is any desired data for initialisation...
		if(initData != nullptr)
		{
			//TODO: Again this needs to addressed properly...
			UINT64 bytes = (format == DXGI_FORMAT_R32_FLOAT) ? sizeof(float) : sizeof(INT8);
			// Give a desc of the data we want to copy
			D3D12_SUBRESOURCE_DATA subResourceData = {};
			subResourceData.pData = initData;
			subResourceData.RowPitch = width * bytes;
			subResourceData.SlicePitch = subResourceData.RowPitch * height;

			// Schedule to copy the data to the default buffer resource.
			// Make instruction to copy CPU buffer into intermediate upload heap
			// buffer.
			pGCL->ResourceBarrier
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
			UpdateSubresources
			(
				pGCL,
				defaultBuffer.Get(),
				uploadBuffer.Get(),
				0,
				0,
				numOfResources,
				&subResourceData
			);

			// Add the instruction to transition back to read 
			pGCL->ResourceBarrier
			(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition
				(
					defaultBuffer.Get(),
					D3D12_RESOURCE_STATE_COPY_DEST,
					D3D12_RESOURCE_STATE_COMMON
				)
			);

		}
		return defaultBuffer;
	}

	ComPtr<ID3D12Resource> D3D12BufferFactory::CreateTexture2D
	(
		UINT32 width,
		UINT32 height,
		UINT16 mipLevels,
		const void* pData,
		DXGI_FORMAT format,
		ComPtr<ID3D12Resource>& uploadBuffer
	)
	{
		const auto device = gD3D12Context->GetDevice();
		CORE_ASSERT(device, "Invalid device!");

		/**
		 * make a wee circle if no data has been passed 
		 */
		HRESULT hr{ S_OK };
		ComPtr<ID3D12Resource> defaultBuffer;

		D3D12_RESOURCE_DESC texDesc{};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = mipLevels;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		//Create the committed resource
		hr = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&defaultBuffer)
		);
		THROW_ON_FAILURE(hr);

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(defaultBuffer.Get(),
			0, texDesc.DepthOrArraySize * texDesc.MipLevels);

		hr = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		);
		THROW_ON_FAILURE(hr);

		if(pData != nullptr)
		{
			// Give a desc of the data we want to copy
			D3D12_SUBRESOURCE_DATA subResourceData = {};
			subResourceData.pData = pData;
			subResourceData.RowPitch = width * sizeof(UINT32);
			subResourceData.SlicePitch = height;// subResourceData.RowPitch* height;

			const UINT32 numOfResources = texDesc.DepthOrArraySize * texDesc.MipLevels;

			pGCL->ResourceBarrier
			(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition
				(
					defaultBuffer.Get(),
					D3D12_RESOURCE_STATE_COMMON,
					D3D12_RESOURCE_STATE_COPY_DEST
				)
			);

			UpdateSubresources
			(
				pGCL,
				defaultBuffer.Get(),
				uploadBuffer.Get(),
				0,
				0,
				numOfResources,
				&subResourceData
			);

			pGCL->ResourceBarrier
			(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition
				(
					defaultBuffer.Get(),
					D3D12_RESOURCE_STATE_COPY_DEST,
					D3D12_RESOURCE_STATE_GENERIC_READ
				)
			);
		}
		
		return defaultBuffer;
	}

	ComPtr<ID3D12Resource> D3D12BufferFactory::CreateRenderTexture(INT32 width, INT32 height, DXGI_FORMAT format)
	{
		const auto device = gD3D12Context->GetDevice();
		CORE_ASSERT(device, "Invalid device!");

		HRESULT hr{ S_OK };

		ComPtr<ID3D12Resource> resource = nullptr;

		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&resource));

		THROW_ON_FAILURE(hr);

		return resource;
	}

	ComPtr<ID3D12Resource> D3D12BufferFactory::CreateCounterResource(bool allowShaderAtomics, bool allowWrite)
	{
		const auto device = gD3D12Context->GetDevice();
		CORE_ASSERT(device, "Invalid device!");

		HRESULT hr{ S_OK };
		ComPtr<ID3D12Resource> counterBuffer = nullptr;
		constexpr UINT64 sizeInBytes = sizeof(UINT32);
		const D3D12_RESOURCE_FLAGS flags = (allowWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
		const D3D12_HEAP_FLAGS heapFlags = (allowShaderAtomics) ? D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS : D3D12_HEAP_FLAG_NONE;

		hr = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			heapFlags,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes, flags),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&counterBuffer)
		);
		THROW_ON_FAILURE(hr);
		return counterBuffer;
	}

	void D3D12BufferFactory::CreateUploadBuffer(ComPtr<ID3D12Resource>& resource, UINT32 bufferWidth)
	{
		const auto device = gD3D12Context->GetDevice();
		CORE_ASSERT(device, "Invalid device!");

		HRESULT hr{ S_OK };
		hr = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(resource.GetAddressOf())
		);
		THROW_ON_FAILURE(hr);
	}

	ComPtr<ID3D12Resource> D3D12BufferFactory::CreateReadBackBuffer(UINT32 bufferWidth)
	{
		const auto device = gD3D12Context->GetDevice();
		CORE_ASSERT(device, "Invalid device!");

		HRESULT hr{ S_OK };
		ComPtr<ID3D12Resource> readBackResource = nullptr;

		hr = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&readBackResource)
		);

		THROW_ON_FAILURE(hr);

		return readBackResource;
	}

	ComPtr<ID3D12Resource> D3D12BufferFactory::CreateStructuredBuffer(UINT32 bufferWidth, bool allowWrite, bool allowAtomics)
	{
		const auto device = gD3D12Context->GetDevice();

		CORE_ASSERT(device, "Invalid device!");

		ComPtr<ID3D12Resource> buffer = nullptr;

		const D3D12_RESOURCE_FLAGS bufferFlags = (allowWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
		const D3D12_RESOURCE_STATES bufferState = (allowWrite) ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_GENERIC_READ;
		const D3D12_HEAP_FLAGS heapFlags = (allowAtomics) ? D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS : D3D12_HEAP_FLAG_NONE;

		const HRESULT result = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			heapFlags,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth, bufferFlags),
			bufferState,
			nullptr,
			IID_PPV_ARGS(&buffer)
		);
		THROW_ON_FAILURE(result);

		return buffer;
		
	}

	ComPtr<ID3D12Resource> D3D12BufferFactory::CreateStructuredBuffer(UINT32 bufferWidth, ID3D12Resource* upload, const void* data, bool allowWrite, bool allowAtomics)
	{
		const auto device = gD3D12Context->GetDevice();

		CORE_ASSERT(device, "Invalid device!");

		HRESULT hr{ S_OK };

		ComPtr<ID3D12Resource> resource = nullptr;

		const D3D12_RESOURCE_FLAGS bufferFlags = (allowWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
		const D3D12_RESOURCE_STATES bufferState = (allowWrite) ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_GENERIC_READ;
		const D3D12_HEAP_FLAGS heapFlags = (allowAtomics) ? D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS : D3D12_HEAP_FLAG_NONE;

		hr = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			heapFlags,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth, bufferFlags),
			bufferState,
			nullptr,
			IID_PPV_ARGS(&resource)
		);
		THROW_ON_FAILURE(hr);

		hr = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&resource)
		);
		THROW_ON_FAILURE(hr);

		D3D12_SUBRESOURCE_DATA srcData = {};
		srcData.pData = data;
		srcData.RowPitch = bufferWidth;
		srcData.SlicePitch = srcData.RowPitch;

		pGCL->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), bufferState,
			D3D12_RESOURCE_STATE_COPY_DEST));

		UpdateSubresources(pGCL, resource.Get(), upload, 0, 0, 1, &srcData);

		pGCL->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
			bufferState));

		return resource;

	}

	UINT D3D12BufferFactory::CalculateBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}
}