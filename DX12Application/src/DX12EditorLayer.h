#pragma once
#include "Framework/Core/Layer/Layer.h"
#include "Framework/DX12Framework.h"
#include "Framework/Camera/Camera.h"

namespace DX12Framework
{
	class DX12EditorLayer : public Layer
	{
	public:

		DX12EditorLayer();
		virtual ~DX12EditorLayer();

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(DeltaTime deltaTime) override;
		void OnRender()override;
		void OnImGuiRender() override;
		void OnEvent(Event& event) override;

	private:


		Camera Camera;
	};

}


