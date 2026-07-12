#include "Engine/Core/Logger.h"

#include <Windows.h>
#include <chrono>
#include <cstdio>

namespace Royal
{
	std::vector<Logger::Sink> Logger::s_sinks;

	void Logger::AddSink(Sink sink)
	{
		s_sinks.push_back(std::move(sink));
	}

	void Logger::LogMessage(LogLevel level, std::source_location location, const std::string& message)
	{
		// Trim full path down to filename.
		std::string_view fullPath = location.file_name();
		size_t lastSlash = fullPath.find_last_of("/\\");
		std::string_view fileName = (lastSlash == std::string_view::npos) ? fullPath : fullPath.substr(lastSlash + 1);

		std::string formatted = std::format("[{}] {}:{} ({}) - {}", ToString(level), fileName, location.line(), location.function_name(), message);

		// Always emit to the debugger/console.
		OutputDebugStringA((formatted + "\n").c_str());
		std::printf("%s\n", formatted.c_str());

		for (auto& sink : s_sinks)
		{
			sink(level, formatted);
		}
	}
}