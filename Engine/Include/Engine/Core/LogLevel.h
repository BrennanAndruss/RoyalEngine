#pragma once

namespace Royal
{
	enum class LogLevel
	{
		Trace,
		Info,
		Warning,
		Error,
		Fatal
	};

	inline const char* ToString(LogLevel level)
	{
		switch (level)
		{
		case LogLevel::Trace: return "TRACE";
		case LogLevel::Info: return "INFO";
		case LogLevel::Warning: return "WARN";
		case LogLevel::Error: return "ERROR";
		case LogLevel::Fatal: return "FATAL";
		}

		return "UNKNOWN";
	}
}