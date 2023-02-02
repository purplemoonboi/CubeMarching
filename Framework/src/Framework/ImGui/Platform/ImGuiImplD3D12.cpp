#include "ImGuiImplD3D12.h"

#include "Framework/Core/Application/Application.h"
#include "Framework/Renderer/Engine/RenderInstruction.h"
#include "Platform/DirectX12/Api/D3D12Context.h"
#include "Platform/DirectX12/Api/D3D12RenderingApi.h"

namespace Engine
{
	ComPtr<ID3D12DescriptorHeap> ImGuiImplD3D12::ImGuiHeap = nullptr;

	void ImGuiImplD3D12::InitialiseImGui
	(
		const UINT32 numOfFramesInFlight
	)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		//io.Fonts->AddFontFromFileTTF("../Foundation-Editor/assets/fonts/worksans/static/WorkSans-Bold.ttf", 18.0f);
		//io.FontDefault = io.Fonts->AddFontFromFileTTF("../Foundation-Editor/assets/fonts/worksans/static/WorkSans-Regular.ttf", 18.0f);

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application* app = Application::Get();
		auto const* window = static_cast<Win32Window*>(app->GetWindow().GetNativeWindow());
		auto const* api = dynamic_cast<D3D12RenderingApi*>(RenderInstruction::GetApiPtr());
		auto const* context = dynamic_cast<D3D12Context*>(api->GetGraphicsContext());
		auto const* frameBuffer = dynamic_cast<D3D12FrameBuffer*>(api->GetFrameBuffer());

		/* create ImGui heap */
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (context->Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&ImGuiHeap)) != S_OK)
		{
		}

		//ImGui_ImplWin32_Init(context->GetHwnd());
		ImGui_ImplWin32_Init(context->GetHwnd());
		ImGui_ImplDX12_Init
		(
			context->Device.Get(),
			numOfFramesInFlight,
			frameBuffer->GetBackBufferFormat(),
			ImGuiHeap.Get(),
			ImGuiHeap->GetCPUDescriptorHandleForHeapStart(),
			ImGuiHeap->GetGPUDescriptorHandleForHeapStart()
		);
	}

	void ImGuiImplD3D12::BeginRenderImpl()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiImplD3D12::EndRenderImpl()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application* app = Application::Get();
		io.DisplaySize = ImVec2(app->GetWindow().GetWidth(), app->GetWindow().GetHeight());

		ImGui::Render();

		auto const* api = dynamic_cast<D3D12RenderingApi*>(RenderInstruction::GetApiPtr());
		auto* context = dynamic_cast<D3D12Context*>(api->GetGraphicsContext());
		auto const* frameBuffer = dynamic_cast<D3D12FrameBuffer*>(api->GetFrameBuffer());

		context->CmdListAlloc->Reset();

		context->GraphicsCmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				frameBuffer->CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			)
		);

		context->GraphicsCmdList->Reset(context->CmdListAlloc.Get(), nullptr);
		// Render Dear ImGui graphics
		//context->GraphicsCmdList->ClearRenderTargetView(frameBuffer->GetCurrentBackBufferViewCpu(), DirectX::Colors::Aqua, 0, NULL);
		context->GraphicsCmdList->OMSetRenderTargets
		(
			1,
			&frameBuffer->GetCurrentBackBufferViewCpu(),
			FALSE,
			NULL
		);

		ID3D12DescriptorHeap* descriptorHeaps[] = { ImGuiHeap.Get() };
		context->GraphicsCmdList->SetDescriptorHeaps(1, descriptorHeaps);

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), context->GraphicsCmdList.Get());

		context->GraphicsCmdList->ResourceBarrier
		(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition
			(
				frameBuffer->CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT
			)
		);
		context->GraphicsCmdList->Close();
		context->CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)context->GraphicsCmdList.Get());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, static_cast<void*>(context->GraphicsCmdList.Get()));
		}

		context->SwapChain->Present(1, 0);
		const UINT64 fenceValue = context->GPU_TO_CPU_SYNC_COUNT + 1;
		context->CommandQueue->Signal(context->Fence.Get(), fenceValue);
		context->GPU_TO_CPU_SYNC_COUNT = fenceValue;
	}
}
