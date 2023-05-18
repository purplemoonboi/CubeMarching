#pragma once

#include <string>
#include "../../vendor/Microsoft/DDSTextureLoader.h"

#include "Platform/DirectX12/Textures/D3D12Texture.h"
#include "Platform/DirectX12/Api/D3D12Context.h"

namespace Foundation
{
	

	class D3D12TextureLoader
	{
	public:

		static void Init(D3D12Context* context)
		{
			Context = context;
		}

		static HRESULT LoadTexture2DFromFile
		(
			const std::wstring& fileName,
			ComPtr<ID3D12Resource>& resource,
			ComPtr<ID3D12Resource>& uploadHeap
		)
		{
			const HRESULT hr = DirectX::CreateDDSTextureFromFile12(
				Context->Device.Get(), 
				Context->ResourceCommandList.Get(), 
				fileName.c_str(),
				resource, uploadHeap
			);
			return hr;
		}

		static inline D3D12Context* Context = nullptr;

	};
}
