#include "D3D12BufferUtils.h"

#include "Framework/Core/Log/Log.h"

#include "Platform/DirectX12/D3D12/D3D12.h"

namespace Foundation::Graphics::D3D12
{

	// Upload buffer methods
	inline ComPtr<ID3D12Resource> D3D12BufferUtils::CreateDefaultBuffer
	(
		const void* initData,
		UINT64 byteSize,
		ComPtr<ID3D12Resource>& uploadBuffer
	)
	{
		ComPtr<ID3D12Resource> defaultBuffer;

		auto pDevice		= gD3D12Context->GetDevice();
		auto pCommandList	= gD3D12Context->GetGraphicsCommandList();

		//Create the committed resource
		HRESULT hr{ S_OK };

		D3D12_HEAP_PROPERTIES hDesc{};
		hDesc.Type					= D3D12_HEAP_TYPE_DEFAULT;
		hDesc.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hDesc.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;
		hDesc.VisibleNodeMask		= 1;
		hDesc.CreationNodeMask		= 1;

		D3D12_RESOURCE_DESC desc;
		desc.Dimension				= D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Flags					= D3D12_RESOURCE_FLAG_NONE;
		desc.Format					= DXGI_FORMAT_UNKNOWN;
		desc.Width					= byteSize;
		desc.Height					= desc.Width;
		desc.DepthOrArraySize		= 1;
		desc.MipLevels				= 0;
		desc.SampleDesc.Count		= 1;
		desc.SampleDesc.Quality		= 0;
		desc.Alignment				= 0;

		hr = pDevice->CreateCommittedResource
		(
			&hDesc,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(defaultBuffer.GetAddressOf())
		);
		THROW_ON_FAILURE(hr);

		hr = pDevice->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		);
		THROW_ON_FAILURE(hr);

		if(initData != nullptr)
		{
			// Give a desc of the data we want to copy
			D3D12_SUBRESOURCE_DATA subResourceData = {};
			subResourceData.pData = initData;
			subResourceData.RowPitch = byteSize;
			subResourceData.SlicePitch = subResourceData.RowPitch;

			D3D12_RESOURCE_BARRIER barrier{};
			ZeroMemory(&barrier, sizeof(D3D12_RESOURCE_BARRIER));
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = defaultBuffer.Get();
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;

			// Schedule to copy the data to the default buffer resource.
			// Make instruction to copy CPU buffer into intermediate upload heap
			// buffer.
			pCommandList->ResourceBarrier(1, &barrier);

			// Copy the data into the upload heap
			UpdateSubresources<1>
				(
					pCommandList,
					defaultBuffer.Get(),
					uploadBuffer.Get(),
					0,
					0,
					1,
					&subResourceData
					);

			
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = defaultBuffer.Get();
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
			
			// Add the instruction to transition back to read 
			pCommandList->ResourceBarrier(1, &barrier);

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
		auto pDevice		= gD3D12Context->GetDevice();
		auto pCommandList	= gD3D12Context->GetGraphicsCommandList();

		ComPtr<ID3D12Resource> defaultBuffer;
		D3D12_RESOURCE_DESC texDesc{};
		texDesc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		texDesc.Alignment			= 0;
		texDesc.Width				= width;
		texDesc.Height				= height;
		texDesc.DepthOrArraySize	= depth;
		texDesc.MipLevels			= mipLevels;
		texDesc.Format				= format;
		texDesc.SampleDesc.Count	= 1;
		texDesc.SampleDesc.Quality	= 0;
		texDesc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags				= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		D3D12_HEAP_PROPERTIES hDesc{};
		hDesc.Type					= D3D12_HEAP_TYPE_DEFAULT;
		hDesc.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hDesc.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;
		hDesc.CreationNodeMask		= 0;
		hDesc.VisibleNodeMask		= 0;

		//Create the committed resource
		const HRESULT defaultResult = pDevice->CreateCommittedResource
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

		const HRESULT uploadResult = pDevice->CreateCommittedResource
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
			pCommandList->ResourceBarrier
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
				pCommandList,
				defaultBuffer.Get(),
				uploadBuffer.Get(),
				0,
				0,
				numOfResources,
				&subResourceData
			);

			// Add the instruction to transition back to read 
			pCommandList->ResourceBarrier
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

		auto pDevice		= gD3D12Context->GetDevice();
		auto pCommandList	= gD3D12Context->GetGraphicsCommandList();

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

		D3D12_HEAP_PROPERTIES hDesc{};
		hDesc.Type					= D3D12_HEAP_TYPE_DEFAULT;
		hDesc.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hDesc.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;
		hDesc.CreationNodeMask		= 0;
		hDesc.VisibleNodeMask		= 0;

		HRESULT hr{ S_OK };

		//Create the committed resource
		hr = pDevice->CreateCommittedResource
		(
			&hDesc,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&defaultBuffer)
		);
		THROW_ON_FAILURE(hr);

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(defaultBuffer.Get(),
			0, texDesc.DepthOrArraySize * texDesc.MipLevels);
	
		hDesc.Type = D3D12_HEAP_TYPE_UPLOAD;

		HRESULT hr = pDevice->CreateCommittedResource
		(
			&hDesc,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		);
		THROW_ON_FAILURE(hr);

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
			pCommandList->ResourceBarrier
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
				pCommandList,
				defaultBuffer.Get(),
				uploadBuffer.Get(),
				0,
				0,
				numOfResources,
				&subResourceData
			);

			// Add the instruction to transition back to read 
			pCommandList->ResourceBarrier
			(
				1,
				&CD3DX12_RESOURCE_BARRIER::Transition
				(
					defaultBuffer.Get(),
					D3D12_RESOURCE_STATE_COPY_DEST,
					D3D12_RESOURCE_STATE_GENERIC_READ
				)
			);

			const HRESULT deviceRemovedReason = pDevice->GetDeviceRemovedReason();
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

		auto pDevice = gD3D12Context->GetDevice();

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

		const HRESULT hr = pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&resource));

		THROW_ON_FAILURE(hr);

		return resource;
	}

	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateCounterResource(bool allowShaderAtomics, bool allowWrite)
	{
		ComPtr<ID3D12Resource> counterBuffer = nullptr;
		constexpr UINT64 sizeInBytes = sizeof(UINT32);
		const D3D12_RESOURCE_FLAGS flags = (allowWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
		const D3D12_HEAP_FLAGS heapFlags = (allowShaderAtomics) ? D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS : D3D12_HEAP_FLAG_NONE;

		auto pDevice = gD3D12Context->GetDevice();

		D3D12_HEAP_PROPERTIES hDesc{};
		hDesc.Type					= D3D12_HEAP_TYPE_DEFAULT;
		hDesc.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hDesc.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;
		hDesc.VisibleNodeMask		= 0;
		hDesc.CreationNodeMask		= 0;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension				= D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Flags					= D3D12_RESOURCE_FLAG_NONE;
		desc.Format					= DXGI_FORMAT_UNKNOWN;
		desc.Width					= 0;
		desc.Height					= desc.Width;
		desc.DepthOrArraySize		= 1;
		desc.MipLevels				= 0;
		desc.SampleDesc.Count		= 1;
		desc.SampleDesc.Quality		= 0;
		desc.Alignment				= 0;

		const HRESULT hr = pDevice->CreateCommittedResource
		(
			&hDesc,
			heapFlags,
			&desc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&counterBuffer)
		);
		THROW_ON_FAILURE(hr);
		return counterBuffer;
	}

	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateReadBackBuffer(UINT32 bufferWidth)
	{
		ComPtr<ID3D12Resource> readBackResource = nullptr;

		auto pDevice = gD3D12Context->GetDevice();

		const HRESULT readBackResult = pDevice->CreateCommittedResource
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

	ComPtr<ID3D12Resource> D3D12BufferUtils::CreateStructuredBuffer(UINT32 bufferWidth, ID3D12Resource* resource, const void* data, bool allowWrite, bool allowAtomics)
	{

		ComPtr<ID3D12Resource> buffer = nullptr;

		const D3D12_RESOURCE_FLAGS bufferFlags = (allowWrite) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
		const D3D12_RESOURCE_STATES bufferState = (allowWrite) ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_GENERIC_READ;
		const D3D12_HEAP_FLAGS heapFlags = (allowAtomics) ? D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS : D3D12_HEAP_FLAG_NONE;
	
		auto pDevice = gD3D12Context->GetDevice();
		auto pCommandList = gD3D12Context->GetGraphicsCommandList();

		D3D12_HEAP_PROPERTIES hDesc{};
		hDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
		hDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		hDesc.VisibleNodeMask = 0;
		hDesc.CreationNodeMask = 0;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Width = 0;
		desc.Height = desc.Width;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Alignment = 0;

		HRESULT hr{ S_OK };
		hr = pDevice->CreateCommittedResource
		(
			&hDesc,
			heapFlags,
			&desc,
			bufferState,
			nullptr,
			IID_PPV_ARGS(&buffer)
		);
		THROW_ON_FAILURE(hr);

		hr = pDevice->CreateCommittedResource
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

		D3D12_RESOURCE_BARRIER barrier{};
		ZeroMemory(&barrier, sizeof(D3D12_RESOURCE_BARRIER));
		barrier.Transition.pResource	= resource;
		barrier.Transition.Subresource	= 0;
		barrier.Transition.StateBefore	= bufferState;
		barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_COPY_DEST;

		pCommandList->ResourceBarrier(1, &barrier);

		UpdateSubresources(pCommandList, buffer.Get(), resource, 0, 0, 1, &srcData);
		barrier.Transition.pResource = resource;
		barrier.Transition.Subresource = 0;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter  = bufferState;
		pCommandList->ResourceBarrier(1, &barrier);

		return buffer;

	}

	UINT D3D12BufferUtils::CalculateConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}
}
