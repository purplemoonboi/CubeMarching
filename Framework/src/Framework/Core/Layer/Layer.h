#pragma once
#include <string>
#include "Framework/Core/Events/Event.h"
#include "Framework/Core/Time/AppTimeManager.h"

namespace Engine
{
	class Layer
	{
	public:
		Layer(const std::wstring& name = L"Layer");
		virtual ~Layer();

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate(const AppTimeManager& timer) = 0;
		virtual void OnRender(const AppTimeManager& timer) = 0;
		virtual void OnImGuiRender() = 0;
		virtual void OnEvent(Event& event) = 0;

		const std::wstring& GetName() { return DebugName; }

	protected:
		std::wstring DebugName;
	};
}



