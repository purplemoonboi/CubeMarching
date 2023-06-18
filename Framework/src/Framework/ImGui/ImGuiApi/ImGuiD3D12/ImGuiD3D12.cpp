#include "ImGuiD3D12.h"
#include "Framework/Core/Application/Application.h"
#include "Framework/Renderer/Renderer3D/RenderInstruction.h"

#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Api/D3D12RenderingApi.h"
#include "Platform/DirectX12/Resources/D3D12RenderFrame.h"
#include "Platform/DirectX12/Buffers/D3D12FrameBuffer.h"
#include "Platform/DirectX12/Core/D3D12Core.h"

#include <imgui.h>

#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

namespace Foundation
{
	using namespace Graphics;
	using namespace D3D12;

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

		auto const* api = RenderInstruction::GetApiPtr();
		auto const* context = dynamic_cast<const D3D12Context*>(api->GetGraphicsContext());
		auto const* frameBuffer = dynamic_cast<const D3D12FrameBuffer*>(api->GetFrameBuffer());

		pHandle = SrvHeap.Allocate();

		ImGui_ImplWin32_Init(context->GetHwnd());
		ImGui_ImplDX12_Init
		(
			pDevice.Get(),
			3,
			frameBuffer->GetBackBufferFormat(),
			SrvHeap.GetHeap(),
			pHandle.CpuHandle,
			pHandle.GpuHandle
		);

		//Create the command allocator 
		HRESULT hr = pDevice->CreateCommandAllocator
		(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(ImGuiAlloc.GetAddressOf())
		);
		THROW_ON_FAILURE(hr);

		//Create the direct command queue
		hr = pDevice->CreateCommandList
		(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			ImGuiAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(ImGuiCommandList.GetAddressOf())
		);
		THROW_ON_FAILURE(hr);

		hr = ImGuiCommandList->Close();
		THROW_ON_FAILURE(hr);
	}

	void ImGuiD3D12::OnRenderImpl()
	{
		HRESULT hr = ImGuiAlloc->Reset();
		THROW_ON_FAILURE(hr);
		hr = ImGuiCommandList->Reset(ImGuiAlloc.Get(), nullptr);
		THROW_ON_FAILURE(hr);

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiD3D12::EndRenderImpl()
	{
		auto const* api = dynamic_cast<D3D12RenderingApi*>(RenderInstruction::GetApiPtr());

		auto* context = dynamic_cast<D3D12Context*>(api->GetGraphicsContext());
		auto* frameBuffer = dynamic_cast<D3D12FrameBuffer*>(api->GetFrameBuffer());

		Application* app = Application::Get();
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(app->GetWindow()->GetWidth(), app->GetWindow()->GetHeight());

		ID3D12DescriptorHeap* descriptorHeaps[] = { SrvHeap.GetHeap() };
		ImGuiCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		frameBuffer->Bind(ImGuiCommandList.Get());

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), ImGuiCommandList.Get());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, ImGuiCommandList.Get());
		}

		frameBuffer->UnBind(ImGuiCommandList.Get());


		HRESULT hr = ImGuiCommandList->Close();
		THROW_ON_FAILURE(hr);

		ID3D12CommandList* cmdList[] = { ImGuiCommandList.Get() };
		context->pQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

		hr = context->pSwapChain->Present(0, 0);
		THROW_ON_FAILURE(hr);

		frameBuffer->SetBackBufferIndex((frameBuffer->GetBackBufferIndex() + 1) % SWAP_CHAIN_BUFFER_COUNT);
		api->GetFrame()->Fence = ++context->SyncCounter;

		hr = context->pQueue->Signal(context->pFence.Get(), api->GetFrame()->Fence);
		THROW_ON_FAILURE(hr);
	}

	void ImGuiD3D12::CleanUp()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}
