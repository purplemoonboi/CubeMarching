#pragma once

#include "Core/Time/DeltaTime.h"
#include "Core/Events/Event.h"

namespace DX12Framework
{
	class Layer
	{
	public:
		Layer(const std::wstring& name = L"Layer");
		virtual ~Layer();

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate(DeltaTime timeStep) = 0;
		virtual void OnRender() = 0;
		virtual void OnImGuiRender() = 0;
		virtual void OnEvent(Event& event) = 0;

		const std::wstring& GetName() { return DebugName; }

	protected:
		std::wstring DebugName;
	};
}



