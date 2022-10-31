#pragma once

namespace Engine
{
	class DeltaTime
	{
	public:

		DeltaTime(float Time = 0.0f)
			:
			Time(Time)
		{
		}

		operator float() const { return Time; }

		float GetSeconds() const { return Time; }

		float GetMilliseconds() const { return Time * 1000.0f; }

	private:
		float Time;
		
	};

}