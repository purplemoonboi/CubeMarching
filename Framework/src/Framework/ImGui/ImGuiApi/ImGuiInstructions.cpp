#include "ImGuiInstructions.h"
#include "ImGuiD3D12/ImGuiD3D12.h"

namespace Foundation
{
	ImGuiApi* ImGuiInstructions::ImGuiPtr = new ImGuiD3D12();

	void ImGuiInstructions::InitialiseImGui()
	{
		ImGuiPtr->InitialiseImGui();
	}

	void ImGuiInstructions::BeginRenderImpl()
	{
		ImGuiPtr->OnRenderImpl();
	}

	void ImGuiInstructions::EndRenderImpl()
	{
		ImGuiPtr->EndRenderImpl();
	}

	void ImGuiInstructions::CleanUp()
	{
		ImGuiPtr->CleanUp();
	}

}
