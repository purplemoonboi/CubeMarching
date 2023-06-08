#pragma once
#include "Platform/DirectX12/DirectX12.h"
#include "Platform/DirectX12/Heap/D3D12HeapManager.h"

#include "../ImGuiApi.h"


namespace Foundation
{

	using Microsoft::WRL::ComPtr;

	class ImGuiD3D12 : public ImGuiApi
	{
	public:

		void InitialiseImGui() override;
		void OnRenderImpl() override;
		void EndRenderImpl() override;
		void CleanUp() override;

	private:
		Graphics::D3D12::D3D12DescriptorHandle pHandle;
		ComPtr<ID3D12CommandAllocator> ImGuiAlloc = nullptr;
		ComPtr<ID3D12GraphicsCommandList> ImGuiCommandList = nullptr;


		UINT64 SignalCount = 0;
	};
}
