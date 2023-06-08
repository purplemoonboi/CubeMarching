#pragma once
#include <string>
#include "Platform/DirectX12/DirectX12.h"

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
		);
		
		static ID3D12GraphicsCommandList4* pGCL;

	};
}
