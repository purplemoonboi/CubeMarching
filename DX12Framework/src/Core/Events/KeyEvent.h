#pragma once
#include "cmpch.h"

#include "Event.h"


namespace DX12Framework
{
	class KeyEvent : public Event
	{
	public:

		INT32 GetKeyCode() const { return KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:

		KeyEvent(INT32 keyCode)
			:
		KeyCode(keyCode)
		{}

		INT32 KeyCode;
	};


	class  KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(INT32 keyCode, INT32 repeatCount)
			:
			KeyEvent(keyCode), RepeatCount(repeatCount)
		{}


		INT32 GetRepeatCount() const { return RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressed: " << KeyCode << " ( " << RepeatCount << " repeats.)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		INT32 RepeatCount;

	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(INT32 keycode)
			:
			KeyEvent(keycode)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTyped: " << KeyCode;

			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};


	class KeyReleasedEvent : public KeyEvent
	{
	public:

		KeyReleasedEvent(INT32 keyCode)
			:
			KeyEvent(keyCode)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << KeyCode;
			return ss.str();
		}


		EVENT_CLASS_TYPE(KeyReleased)
	};
}