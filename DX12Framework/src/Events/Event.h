#pragma once
#include "cmpch.h"
#include "Core/Core.h"


namespace DX12Framework
{
	enum class EventType
	{
		None = 0,

		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScroll
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication	= BIT(0),
		EventCategoryInput			= BIT(1),
		EventCategoryKeyboard       = BIT(2),
		EventCategoryMouse			= BIT(3),
		EventCategoryMouseButton	= BIT(4)
	};

#define EVENT_CLASS_TYPE(type) static  EventType   GetStaticType() {return EventType::##type; }\
							   virtual EventType   GetEventType() const override { return GetStaticType(); }\
							   virtual const char* GetName() const override { return #type; }


#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }


	class Event
	{
		friend class EventDispatcher;

	public:

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName()    const = 0;
		virtual int GetCategoryFlags()   const = 0;
		virtual std::string ToString()   const { return GetName(); }
		const bool HasEventBeenHandled() { return IsHandled; }

		// @brief Bitwise '&' to evaluate if 'this' event --(the event we're processing)-- is in the category.
		bool InCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}


		
		bool IsHandled = false;

	};


	// @brief 
	class EventDispatcher
	{
		template<typename T>
		using EventAction = std::function<bool(T&)>;

	public:

		EventDispatcher(Event& event)
			:
			Event(event)
		{}

		template<typename T> bool Dispatch(EventAction<T> function)
		{
			if (Event.GetEventType() == T::GetStaticType())
			{
				Event.IsHandled = function(*(T*)&Event);
				return true;
			}
			return false;
		}

	private:
		Event& Event;
	};



	inline std::ostream& operator << (std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}

}