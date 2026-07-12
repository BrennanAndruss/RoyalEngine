#pragma once

#include <chrono>

namespace Royal
{
	class Time
	{
	public:
		static void Init();
		static void Tick();

		static float GetDeltaTime() { return s_deltaTime; }
		static float GetTimeSeconds() { return s_totalTime; }

	private:
		using Clock = std::chrono::steady_clock;
		static Clock::time_point s_startTime;
		static Clock::time_point s_lastFrameTime;
		static float s_deltaTime;
		static float s_totalTime;
	};
}