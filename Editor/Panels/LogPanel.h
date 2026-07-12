#pragma once

#include "Engine/Core/LogLevel.h"

#include <deque>
#include <string>

namespace Editor::Panels
{
	class LogPanel
	{
	public:
		LogPanel(const LogPanel&) = delete;
		LogPanel& operator=(const LogPanel&) = delete;

		static LogPanel& Get();

		void AddLog(Royal::LogLevel level, const std::string& message);
		void Draw(bool& open);

	private:
		LogPanel() = default;

		struct LogEntry
		{
			Royal::LogLevel level;
			std::string message;
		};

		static constexpr size_t kMaxLines = 500;
		std::deque<LogEntry> m_entries;
		bool m_autoScroll = true;

		// Filter toggles.
		bool m_showInfo = true;
		bool m_showWarning = true;
		bool m_showError = true;

		bool PassesFilter(Royal::LogLevel level) const;
	};
}