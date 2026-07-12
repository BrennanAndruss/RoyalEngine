#pragma once

#include "Engine/Core/Logger.h"

#include <source_location>

namespace Royal
{

#define ROYAL_DEBUG_BREAK() __debugbreak()

}

#if defined(_DEBUG)

#define ROYAL_ASSERT(condition, ...)													\
do																						\
{																						\
	if (!(condition))																	\
	{																					\
		::Royal::Logger::Log(::Royal::LogLevel::Fatal, std::source_location::current(), \
			"Assertion failed: ({}) " #condition, ##__VA_ARGS__);						\
		ROYAL_DEBUG_BREAK();															\
	}																					\
} while (0)

#else

#define ROYAL_ASSERT(condition, ...) ((void)0)

#endif

#define ROYAL_VERIFY(condition, ...)													\
( (condition) ? true : (																\
	::Royal::Logger::Log(::Royal::LogLevel::Error, std::source_location::current(),		\
		"Verify failed: ({}) " #condition, ##__VA_ARGS__),								\
		false																			\
		) )