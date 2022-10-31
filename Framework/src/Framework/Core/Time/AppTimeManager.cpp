#include "Framework/cmpch.h"
#include "AppTimeManager.h"

AppTimeManager::AppTimeManager()
	:
	SecondPerCount(0.0),
	BaseTime(0),
	PausedTime(0),
	PreviousTime(0),
	CurrentTime(0),
	HasStopped(false)
{
	INT64 countsPerSecond;

	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);
	SecondPerCount = 1.0 / static_cast<double>(countsPerSecond);
}

float AppTimeManager::TimeElapsed() const
{
	//If the application has been paused, don't track the time passed since.
	//This carries forward in future pause events, hence subtract pause time from
	//stop!
	if(HasStopped)
	{
		return static_cast<float>((StopTime - PausedTime) - BaseTime * SecondPerCount);
	}

	//Similarly, subtract the pause time from current time before subtracting applications t0.
	return static_cast<float>(((CurrentTime - PausedTime) - BaseTime) * SecondPerCount);
}


void AppTimeManager::Reset()
{
	INT64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	BaseTime = currTime;
	PreviousTime = currTime;
	StopTime = 0;
	HasStopped = false;
}

void AppTimeManager::Start()
{
	INT64 startTime;
	QueryPerformanceFrequency((LARGE_INTEGER*)&startTime);


	if(HasStopped)
	{
		PausedTime += (startTime - StopTime);
		PreviousTime = startTime;
		StopTime = 0;
		HasStopped = false;
	}
}

void AppTimeManager::Stop()
{
	if (!HasStopped)
	{
		INT64 currentTime;
		StopTime = currentTime;
		HasStopped = true;
	}
}

void AppTimeManager::Tick()
{
	if(HasStopped)
	{
		Time = 0.0f;
	}

	INT64 currentTime;
	QueryPerformanceFrequency((LARGE_INTEGER*)&currentTime);

	CurrentTime = currentTime;
	//Delta from last frame
	Time = (CurrentTime - PreviousTime) * SecondPerCount;
	PreviousTime = CurrentTime;

	//DXSDK mentions if the CPU enters power save mode or the current process
	//is moved to a another processor, delta time can become negative.
	if(Time < 0.0f)
	{
		Time = 0.0;
	}
}



