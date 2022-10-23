#pragma once
#include "DirectX12.h"


namespace DX12Framework
{
	class DX12ResourceManager
	{
	public:

		// @brief Creates the description for a resource buffer. (of type DSV)
		bool CreateDsvDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device);

		// @brief Creates the description for a resource buffer. (of type RTV)
		bool CreateRtvDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device);


	};

}

