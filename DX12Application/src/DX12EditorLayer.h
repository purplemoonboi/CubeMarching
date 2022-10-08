#pragma once
#include "Core/Layer/Layer.h"
#include "DX12Framework.h"

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

		//struct ProfilingResults
		//{
		//	const char* name;
		//	float time;
		//};
		//std::vector<ProfilingResults> m_Profiles;
	};

}


