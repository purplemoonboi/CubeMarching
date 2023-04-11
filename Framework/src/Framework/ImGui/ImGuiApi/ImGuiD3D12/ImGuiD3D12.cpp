#include "ImGuiD3D12.h"
#include "Framework/Core/Application/Application.h"
#include "Framework/Renderer/Engine/RenderInstruction.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Api/D3D12RenderingApi.h"

#include <imgui.h>

#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

namespace Engine
{
	void ImGuiD3D12::InitialiseImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		io.Fonts->AddFontFromFileTTF("assets/fonts/worksans/static/WorkSans-Bold.ttf", 18.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/worksans/static/WorkSans-Regular.ttf", 18.0f);

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		auto const* api = dynamic_cast<D3D12RenderingApi*>(RenderInstruction::GetApiPtr());
		auto const* context = dynamic_cast<D3D12Context*>(api->GetGraphicsContext());
		auto const* frameBuffer = dynamic_cast<D3D12FrameBuffer*>(api->GetFrameBuffer());
		auto const* memoryManager = dynamic_cast<D3D12MemoryManager*>(api->GetMemoryManager());

		ImGui_ImplWin32_Init(context->GetHwnd());
		ImGui_ImplDX12_Init
		(
			context->Device.Get(),
			3,
			frameBuffer->GetBackBufferFormat(),
			memoryManager->GetShaderResourceDescHeap(),
			memoryManager->GetImGuiHandles()->CpuHandle,
			memoryManager->GetImGuiHandles()->GpuHandle
		);

		//Create the command allocator 
		const HRESULT cmdQueueAllocResult = context->Device->CreateCommandAllocator
		(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(ImGuiAlloc.GetAddressOf())
		);
		THROW_ON_FAILURE(cmdQueueAllocResult);

		//Create the direct command queue
		const HRESULT cmdListResult = context->Device->CreateCommandList
		(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			ImGuiAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(ImGuiCmdList.GetAddressOf())
		);
		THROW_ON_FAILURE(cmdListResult);

		const HRESULT hh = ImGuiCmdList->Close();
		THROW_ON_FAILURE(hh);
	}

	void ImGuiD3D12::OnRenderImpl()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiD3D12::EndRenderImpl()
	{
		auto const* api = dynamic_cast<D3D12RenderingApi*>(RenderInstruction::GetApiPtr());
		auto const* memoryManager = dynamic_cast<D3D12MemoryManager*>(api->GetMemoryManager());

		auto* context = dynamic_cast<D3D12Context*>(api->GetGraphicsContext());
		auto* frameBuffer = dynamic_cast<D3D12FrameBuffer*>(api->GetFrameBuffer());

		Application* app = Application::Get();
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(app->GetWindow().GetWidth(), app->GetWindow().GetHeight());

		frameBuffer->Bind();

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), context->GraphicsCmdList.Get());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, context->GraphicsCmdList.Get());
		}

		frameBuffer->UnBind();


		const HRESULT closeResult = context->GraphicsCmdList->Close();
		THROW_ON_FAILURE(closeResult);

		ID3D12CommandList* cmdList[] = { context->GraphicsCmdList.Get() };
		context->CommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

		const HRESULT presentResult = context->SwapChain->Present(0, 0);
		THROW_ON_FAILURE(presentResult);

		frameBuffer->SetBackBufferIndex((frameBuffer->GetBackBufferIndex() + 1) % SWAP_CHAIN_BUFFER_COUNT);
		api->GetCurrentFrameResource()->Fence = ++context->SyncCounter;

		const HRESULT signalResult = context->CommandQueue->Signal(context->Fence.Get(),
			api->GetCurrentFrameResource()->Fence);
		THROW_ON_FAILURE(signalResult);
	}

	void ImGuiD3D12::CleanUp()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}
