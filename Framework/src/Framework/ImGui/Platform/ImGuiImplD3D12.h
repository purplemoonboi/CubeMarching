#pragma once
#include <intsafe.h>

#include <Platform/DirectX12/DirectX12.h>

namespace Engine
{
	class D3D12MemoryManager;
	using Microsoft::WRL::ComPtr;

	/** forward declarations */
	class D3D12Context;

	class ImGuiImplD3D12
	{
	public:
		/**
		 * @brief Initialises ImGui using direct x12 api. Also creates a heap for ImGui. 
		 * @param context A pointer to the direct x12 api high level obj containing a ptr to the device.
		 * @param numOfFramesInFlight The number of frames in flight.
		 */
		static void InitialiseImGui
		(
			const UINT32 numOfFramesInFlight
		);


		static void BeginRenderImpl();

		static void EndRenderImpl();

		static void CleanUp();
	};
}