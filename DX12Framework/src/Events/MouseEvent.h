#pragma once

#include "cmpch.h"
#include "Event.h"


namespace DX12Framework
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y)
			: 
			MouseX(x), MouseY(y) {}

		float GetXCoordinate() const { return MouseX; }
		float GetYCoordinate() const { return MouseY; }

		std::string ToString() const override
		{
			std::stringstream StringStream;
			StringStream << "MouseMovedEvent: " << MouseX << ", " << MouseY;
			return StringStream.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:

		float MouseX, MouseY;
	};


	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float x, float y)
			: 
			MouseX(x), MouseY(y) {}

		float GetXOffset() const { return MouseX; }
		float GetYOffset() const { return MouseY; }

		std::string ToString() const override
		{
			std::stringstream StringStream;
			StringStream << "MouseScrolledEvent: " << MouseX << ", " << MouseY;
			return StringStream.str();
		}

		EVENT_CLASS_TYPE(MouseScroll)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float MouseX, MouseY;
	};

	class MouseButtonEvent : public Event
	{
	public:
		INT32 GetMouseButton() const { return MouseButtonCode; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	protected:

		MouseButtonEvent(){}

		MouseButtonEvent(INT32 button)
			:
			MouseButtonCode(button) {}

		INT32 MouseButtonCode;
	};


	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(INT32 button)
			:
			MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream StringStream;
			StringStream << "MouseButtonPressed: " << MouseButtonCode;
			return StringStream.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(INT32 button)
			: 
			MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream StringStream;
			StringStream << "MouseButtonReleased: " << MouseButtonCode;
			return StringStream.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}