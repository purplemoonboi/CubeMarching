#pragma once

class AppTimeManager
{
public:
	AppTimeManager();
	AppTimeManager(const AppTimeManager&) = delete;

	double TimeElapsed() const;
	double DeltaTime() const { return DTime; };

	void Reset();
	void Start();
	void Stop();
	void Tick();


	double GetMilliseconds() const { return DTime * 1000.0; }

private:
	double DTime;
	double SecondPerCount;

	INT64 BaseTime;
	INT64 PausedTime;
	INT64 StopTime;
	INT64 PreviousTime;
	INT64 CurrentTime;

	bool HasStopped;

};

