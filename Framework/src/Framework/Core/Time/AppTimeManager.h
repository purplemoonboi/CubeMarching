#pragma once
#include <intsafe.h>

namespace Foundation
{
	class AppTimeManager
	{
	public:
		AppTimeManager();
		//AppTimeManager(const AppTimeManager&) = delete;

		float TimeElapsed() const;
		float DeltaTime() const { return DTime; }

		void Reset();
		void Start();
		void Stop();
		void Tick();


		double GetMilliseconds() const { return static_cast<double>(DTime) * 1000.0; }

	private:
		float DTime;
		float SecondPerCount;

		INT64 BaseTime;
		INT64 PausedTime;
		INT64 StopTime;
		INT64 PreviousTime;
		INT64 CurrentTime;

		bool HasStopped;

	};
}


