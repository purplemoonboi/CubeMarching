#pragma once

class AppTimeManager
{
public:
	AppTimeManager();
	AppTimeManager(const AppTimeManager&) = delete;

	float TimeElapsed() const;
	float DeltaTime() const { return Time; };

	void Reset();
	void Start();
	void Stop();
	void Tick();


	operator float() const { return Time; }

	float GetSeconds() const { return Time; }

	float GetMilliseconds() const { return Time * 1000.0f; }

private:
	float Time;
	double SecondPerCount;

	INT64 BaseTime;
	INT64 PausedTime;
	INT64 StopTime;
	INT64 PreviousTime;
	INT64 CurrentTime;

	bool HasStopped;

};

