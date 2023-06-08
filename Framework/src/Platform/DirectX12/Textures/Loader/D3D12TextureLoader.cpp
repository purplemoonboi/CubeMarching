#include "D3D12TextureLoader.h"
#include "Platform/DirectX12/Core/D3D12Core.h"
#include <DDSTextureLoader.h>


namespace Foundation::Graphics::D3D12
{

	ID3D12GraphicsCommandList4* D3D12TextureLoader::pGCL { nullptr };

	HRESULT D3D12TextureLoader::LoadTexture2DFromFile(
		const std::wstring& fileName,
		ComPtr<ID3D12Resource>& resource,
		ComPtr<ID3D12Resource>& uploadHeap
	)
	{
		const HRESULT hr = DirectX::CreateDDSTextureFromFile12(
			pDevice.Get(),
			pGCL,
			fileName.c_str(),
			resource, uploadHeap
		);
		return hr;
	}

}
