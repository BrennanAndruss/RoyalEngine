#pragma once

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

		void AddLog(const std::string& message);
		void Draw(bool& open);

	private:
		LogPanel() = default;

		static constexpr size_t kMaxLines = 500;
		std::deque<std::string> m_lines;
		bool m_autoScroll = true;
	};
}