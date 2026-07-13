#include "Engine/Core/Profiler.h"

namespace Royal
{
	Profiler& Profiler::Get()
	{
		static Profiler instance;
		return instance;
	}

	void Profiler::BeginFrame()
	{
		m_lastFrameSamples = std::move(m_currentFrameSamples);
		m_currentFrameSamples.clear();
	}

	void Profiler::RecordCPUSample(const std::string& name, float ms)
	{
		m_currentFrameSamples.push_back({ name, ms });
	}
}