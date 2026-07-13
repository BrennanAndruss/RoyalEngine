#pragma once

#include <chrono>
#include <string>
#include <vector>

namespace Royal
{
	struct CPUSample
	{
		std::string name;
		float milliseconds;
	};

	class Profiler
	{
	public:
		static Profiler& Get();

		void BeginFrame();
		void RecordCPUSample(const std::string& name, float ms);
		const std::vector<CPUSample>& GetLastFrameCPUSamples() const { return m_lastFrameSamples; }

	private:
		std::vector<CPUSample> m_currentFrameSamples;
		std::vector<CPUSample> m_lastFrameSamples;
	};

	class ScopedCPUTimer
	{
	public:
		explicit ScopedCPUTimer(std::string name)
			: m_name(std::move(name)), m_start(Clock::now()) {}

		~ScopedCPUTimer()
		{
			float ms = std::chrono::duration<float, std::milli>(Clock::now() - m_start).count();
			Profiler::Get().RecordCPUSample(m_name, ms);
		}

	private:
		using Clock = std::chrono::steady_clock;
		std::string m_name;
		Clock::time_point m_start;
	};
}

#define ROYAL_CONCAT_INNER(a, b) a##b
#define ROYAL_CONCAT(a, b) ROYAL_CONCAT_INNER(a, b)
#define ROYAL_PROFILE_SCOPE(name) ::Royal::ScopedCPUTimer ROYAL_CONCAT(_profileScope, __LINE__)(name)