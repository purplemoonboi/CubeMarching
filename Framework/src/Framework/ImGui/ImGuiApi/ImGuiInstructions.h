#pragma once
#include <intsafe.h>

#include <Platform/DirectX12/DirectX12.h>

namespace Engine
{
	class D3D12HeapManager;
	using Microsoft::WRL::ComPtr;

	/** forward declarations */
	class ImGuiApi;

	class ImGuiInstructions
	{
	public:
		/**
		 * @brief Initialises ImGui using direct x12 api. Also creates a heap for ImGui. 
		 * @param context A pointer to the direct x12 api high level obj containing a ptr to the device.
		 * @param numOfFramesInFlight The number of frames in flight.
		 */
		static void InitialiseImGui();

		static void BeginRenderImpl();

		static void EndRenderImpl();

		static void CleanUp();

	private:

		static ImGuiApi* ImGuiPtr;

	};
}