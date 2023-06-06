#pragma once

#include <string>
#include "../../vendor/Microsoft/DDSTextureLoader.h"

#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Foundation::Graphics::D3D12
{
	

	class D3D12TextureLoader
	{
	public:

		static void Init(ID3D12GraphicsCommandList4* pCmd)
		{
			pGCL = pCmd;
		}

		static HRESULT LoadTexture2DFromFile
		(
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

		static inline ID3D12GraphicsCommandList4* pGCL{ nullptr };

	};
}
