#pragma once
#include "Platform/DirectX12/DirectX12.h"
#include "../ImGuiApi.h"

namespace Engine
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

		ComPtr<ID3D12CommandAllocator> ImGuiAlloc = nullptr;
		ComPtr<ID3D12GraphicsCommandList> ImGuiCommandList = nullptr;


		UINT64 SignalCount = 0;
	};
}