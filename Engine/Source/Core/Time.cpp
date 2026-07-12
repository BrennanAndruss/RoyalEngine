#include "Engine/Core/Time.h"

namespace Royal
{
	Time::Clock::time_point Time::s_startTime;
	Time::Clock::time_point Time::s_lastFrameTime;
	float Time::s_deltaTime = 0.0f;
	float Time::s_totalTime = 0.0f;

	void Time::Init()
	{
		s_startTime = Clock::now();
		s_lastFrameTime = s_startTime;
	}

	void Time::Tick()
	{
		Clock::time_point now = Clock::now();
		s_deltaTime = std::chrono::duration<float>(now - s_lastFrameTime).count();
		s_totalTime = std::chrono::duration<float>(now - s_startTime).count();
		s_lastFrameTime = now;
	}
}