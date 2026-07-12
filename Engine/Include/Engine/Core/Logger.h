#pragma once

#include "Engine/Core/LogLevel.h"

#include <format>
#include <functional>
#include <source_location>
#include <string>
#include <vector>

namespace Royal
{
	class Logger
	{
	public:
		using Sink = std::function<void(LogLevel, const std::string& formattedMessage)>;

		static void AddSink(Sink sink);

		static void LogMessage(LogLevel level, std::source_location location, const std::string& message);

		template<typename... Args>
		static void Log(LogLevel level, std::source_location location, std::format_string<Args...> format, Args&&... args)
		{
			LogMessage(level, location, std::format(format, std::forward<Args>(args)...));
		}

	private:
		static std::vector<Sink> s_sinks;
	};
}

// Macros capture source_location at the call site automatically.
#define ROYAL_LOG_TRACE(format, ...) ::Royal::Logger::Log(::Royal::LogLevel::Trace, std::source_location::current(), format, ##__VA_ARGS__)
#define ROYAL_LOG_INFO(format, ...) ::Royal::Logger::Log(::Royal::LogLevel::Info, std::source_location::current(), format, ##__VA_ARGS__)
#define ROYAL_LOG_WARN(format, ...) ::Royal::Logger::Log(::Royal::LogLevel::Warning, std::source_location::current(), format, ##__VA_ARGS__)
#define ROYAL_LOG_ERROR(format, ...) ::Royal::Logger::Log(::Royal::LogLevel::Error, std::source_location::current(), format, ##__VA_ARGS__)
#define ROYAL_LOG_FATAL(format, ...) ::Royal::Logger::Log(::Royal::LogLevel::Fatal, std::source_location::current(), format, ##__VA_ARGS__)