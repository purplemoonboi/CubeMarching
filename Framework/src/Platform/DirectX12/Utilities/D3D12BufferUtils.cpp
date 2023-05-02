#include "D3D12BufferUtils.h"

#include "Framework/Core/Log/Log.h"

namespace Engine
{

	ID3D12Device* D3D12BufferUtils::Device = nullptr;
	ID3D12GraphicsCommandList* D3D12BufferUtils::GraphicsCmdList = nullptr;

	void D3D12BufferUtils::Init(ID3D12Device* device, ID3D12GraphicsCommandList* graphicsCmdList)
	{
		Device = device;
		GraphicsCmdList = graphicsCmdList;
	}


	// Upload buffer methods
	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateDefaultBuffer
	(
		const void* initData,
		UINT64 byteSize,
		ComPtr<ID3D12Resource>& uploadBuffer
	)
	{
		ComPtr<ID3D12Resource> defaultBuffer;

		const HRESULT deviceCr = Device->GetDeviceRemovedReason();
		THROW_ON_FAILURE(deviceCr);

		//Create the committed resource
		const HRESULT vertexResult = Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(defaultBuffer.GetAddressOf())
		);
		THROW_ON_FAILURE(vertexResult);

		const HRESULT uploadResult = Device->CreateCommittedResource
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
			GraphicsCmdList->ResourceBarrier
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
					GraphicsCmdList,
					defaultBuffer.Get(),
					uploadBuffer.Get(),
					0,
					0,
					1,
					&subResourceData
					);

			// Add the instruction to transition back to read 
			GraphicsCmdList->ResourceBarrier
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



	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateTexture3D
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
		const HRESULT defaultResult = Device->CreateCommittedResource
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

		const HRESULT uploadResult = Device->CreateCommittedResource
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
			GraphicsCmdList->ResourceBarrier
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
				GraphicsCmdList,
				defaultBuffer.Get(),
				uploadBuffer.Get(),
				0,
				0,
				numOfResources,
				&subResourceData
			);

			// Add the instruction to transition back to read 
			GraphicsCmdList->ResourceBarrier
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

	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateTexture2D
	(
		UINT32 width,
		UINT32 height,
		UINT16 mipLevels,
		const void* initData,
		DXGI_FORMAT format,
		ComPtr<ID3D12Resource>& uploadBuffer
	)
	{
		/**
		 * make a wee circle if no data has been passed 
		 */
		
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
		const HRESULT defaultResult = Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&defaultBuffer)
		);
		THROW_ON_FAILURE(defaultResult);

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(defaultBuffer.Get(),
			0, texDesc.DepthOrArraySize * texDesc.MipLevels);
		HRESULT uploadResult = Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
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
			subResourceData.RowPitch = width * sizeof(UINT32);
			subResourceData.SlicePitch = height;// subResourceData.RowPitch* height;

			const UINT32 numOfResources = texDesc.DepthOrArraySize * texDesc.MipLevels;


			// Schedule to copy the data to the default buffer resource.
			// Make instruction to copy CPU buffer into intermediate upload heap
			// buffer.
			GraphicsCmdList->ResourceBarrier
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
				GraphicsCmdList,
				defaultBuffer.Get(),
				uploadBuffer.Get(),
				0,
				0,
				numOfResources,
				&subResourceData
			);

			// Add the instruction to transition back to read 
			GraphicsCmdList->ResourceBarrier
			(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition
				(
					defaultBuffer.Get(),
					D3D12_RESOURCE_STATE_COPY_DEST,
					D3D12_RESOURCE_STATE_GENERIC_READ
				)
			);

			const HRESULT deviceRemovedReason = Device->GetDeviceRemovedReason();
			THROW_ON_FAILURE(deviceRemovedReason);

			// IMPORTANT: The upload buffer must be kept in scope after the above function calls. This is
			//			  because the cmd list has NOT executed the copy.
			//
			//			  The buffer can be released after the caller knows the copy has been made.
		}

		
		return defaultBuffer;
	}

	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateRenderTexture(INT32 width, INT32 height, DXGI_FORMAT format)
	{
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

		const HRESULT hr = Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&resource));

		THROW_ON_FAILURE(hr);

		return resource;
	}

	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateShadowMap(INT32 width, INT32 height)
	{
		ComPtr<ID3D12Resource> shadowMap;

		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		THROW_ON_FAILURE(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&optClear,
			IID_PPV_ARGS(&shadowMap)));

		return shadowMap;
	}

	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateCounterResource(bool allowShaderAtomics, bool allowWrite)
	{
		ComPtr<ID3D12Resource> counterBuffer = nullptr;
		constexpr UINT64 sizeInBytes = sizeof(UINT32);
		const D3D12_RESOURCE_FLAGS flags = (allowWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
		const D3D12_HEAP_FLAGS heapFlags = (allowShaderAtomics) ? D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS : D3D12_HEAP_FLAG_NONE;

		const HRESULT counterResult = Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			heapFlags,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes, flags),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&counterBuffer)
		);
		THROW_ON_FAILURE(counterResult);
		return counterBuffer;
	}

	void D3D12BufferUtils::CreateUploadBuffer(ComPtr<ID3D12Resource>& resource, UINT32 bufferWidth)
	{
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

	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateReadBackBuffer(UINT32 bufferWidth)
	{
		ComPtr<ID3D12Resource> readBackResource = nullptr;

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

		return readBackResource;
	}

	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateReadBackTex3D(DXGI_FORMAT format, INT32 width, INT32 height, INT32 depth)
	{
		ComPtr<ID3D12Resource> readBackResource = nullptr;

		const HRESULT readBackResult = Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex3D(format, width, height, depth),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&readBackResource)
		);

		THROW_ON_FAILURE(readBackResult);

		return readBackResource;
	}

	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateReadBackTex2D(DXGI_FORMAT format, INT32 width, INT32 height)
	{
		ComPtr<ID3D12Resource> readBackResource = nullptr;

		const HRESULT readBackResult = Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(format, width, height),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&readBackResource)
		);

		THROW_ON_FAILURE(readBackResult);

		return readBackResource;
	}


	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateStructuredBuffer(UINT32 bufferWidth, bool allowWrite, bool allowAtomics)
	{
		
		ComPtr<ID3D12Resource> buffer = nullptr;

		const D3D12_RESOURCE_FLAGS bufferFlags = (allowWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
		const D3D12_RESOURCE_STATES bufferState = (allowWrite) ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_GENERIC_READ;
		const D3D12_HEAP_FLAGS heapFlags = (allowAtomics) ? D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS : D3D12_HEAP_FLAG_NONE;

		const HRESULT result = Device->CreateCommittedResource
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

	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateStructuredBuffer(UINT32 bufferWidth, ID3D12Resource* resource, const void* data, bool allowWrite, bool allowAtomics)
	{

		ComPtr<ID3D12Resource> buffer = nullptr;

		const D3D12_RESOURCE_FLAGS bufferFlags = (allowWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
		const D3D12_RESOURCE_STATES bufferState = (allowWrite) ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_GENERIC_READ;
		const D3D12_HEAP_FLAGS heapFlags = (allowAtomics) ? D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS : D3D12_HEAP_FLAG_NONE;

		const HRESULT result = Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			heapFlags,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth, bufferFlags),
			bufferState,
			nullptr,
			IID_PPV_ARGS(&buffer)
		);
		THROW_ON_FAILURE(result);

		const HRESULT uploadResult = Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferWidth),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&resource)
		);
		THROW_ON_FAILURE(uploadResult);

		D3D12_SUBRESOURCE_DATA srcData = {};
		srcData.pData = data;
		srcData.RowPitch = bufferWidth;
		srcData.SlicePitch = srcData.RowPitch;

		GraphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), bufferState,
			D3D12_RESOURCE_STATE_COPY_DEST));

		UpdateSubresources(GraphicsCmdList, buffer.Get(), resource, 0, 0, 1, &srcData);

		GraphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
			bufferState));

		return buffer;

	}

	UINT D3D12BufferUtils::CalculateConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}
}
