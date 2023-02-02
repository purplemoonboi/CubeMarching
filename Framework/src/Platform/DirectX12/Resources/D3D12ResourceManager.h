#pragma once
#include <vector>

#include "../DirectX12.h"


namespace Engine
{
	class D3D12Texture;
	class D3D12ResourceManager
	{
	public:
		D3D12ResourceManager() = default;
		D3D12ResourceManager(ID3D12Device* device);


		// @brief Creates the descriptors for shader resources
		bool CreateSRVDescriptorHeap
		(
			UINT count, 
			const std::vector<D3D12Texture>& resources
		);


	private:

		ID3D12Device* Device = nullptr;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CbvSrvUavDescriptorHeap = nullptr;
		UINT CbvSrvUavDescriptorSize = 0;
	};

}

