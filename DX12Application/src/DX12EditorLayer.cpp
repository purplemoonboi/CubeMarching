#include "DX12EditorLayer.h"
#include "Framework/Renderer/RenderInstruction.h"


namespace DX12Framework
{
    DX12EditorLayer::DX12EditorLayer()
        :
        Layer(L"Scene Editor")
    {
    }

    DX12EditorLayer::~DX12EditorLayer()
    {
    }

    void DX12EditorLayer::OnAttach()
    {
    }

    void DX12EditorLayer::OnDetach()
    {
    }

    void DX12EditorLayer::OnUpdate(DeltaTime deltaTime)
    {
        //User input


        //Update camera transform


    }

    void DX12EditorLayer::OnRender()
    {

        //TODO: Need to think about a more delicate way of doing this.
        RenderInstruction::Draw();

    }

    void DX12EditorLayer::OnImGuiRender()
    {
    }

    void DX12EditorLayer::OnEvent(Event& event)
    {
    }

}
