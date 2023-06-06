#pragma once
#include <string>
#include "Framework/Core/Events/Event.h"


namespace Foundation
{
	class AppTimeManager;

	class Layer
	{
	public:
		Layer(const std::wstring& name = L"Layer");
		virtual ~Layer();

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate(AppTimeManager* time) = 0;
		virtual void OnRender(AppTimeManager* time) = 0;
		virtual void OnImGuiRender() = 0;
		virtual void OnEvent(Event& event) = 0;

		const std::wstring& GetName() { return DebugName; }

	protected:
		/*virtual void OnPreAttach() = 0;
		virtual void OnPreDetach() = 0;*/

		std::wstring DebugName;
	};
}



