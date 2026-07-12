#include "Panels/LogPanel.h"

#include <imgui.h>
#include <chrono>
#include <format>

namespace Editor::Panels
{
	static std::string CurrentTimestamp()
	{
		const auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
		std::chrono::zoned_time zonedTime{ std::chrono::current_zone(), now };
		return std::format("[{:%H:%M:%S}]", zonedTime.get_local_time());
	}

	LogPanel& LogPanel::Get()
	{
		static LogPanel instance;
		return instance;
	}

	void LogPanel::AddLog(Royal::LogLevel level, const std::string& message)
	{
		m_entries.push_back({ level, CurrentTimestamp() + " " + message });
		if (m_entries.size() > kMaxLines)
		{
			m_entries.pop_front();
		}
	}

	bool LogPanel::PassesFilter(Royal::LogLevel level) const
	{
		switch (level)
		{
		case Royal::LogLevel::Trace:
		case Royal::LogLevel::Info:
			return m_showInfo;
		case Royal::LogLevel::Warning:
			return m_showWarning;
		case Royal::LogLevel::Error:
		case Royal::LogLevel::Fatal:
			return m_showError;
		}

		return true;
	}

	void LogPanel::Draw(bool& open)
	{
		if (!open) return;

		if (ImGui::Begin("Log", &open))
		{
			if (ImGui::Button("Clear"))
			{
				m_entries.clear();
			}
			ImGui::SameLine();
			ImGui::Checkbox("Auto-scroll", &m_autoScroll);

			ImGui::SameLine();

			ImGui::Checkbox("Info", &m_showInfo);
			ImGui::SameLine();
			ImGui::Checkbox("Warning", &m_showWarning);
			ImGui::SameLine();
			ImGui::Checkbox("Error", &m_showError);

			ImGui::Separator();

			ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
			
			for (const auto& entry : m_entries)
			{
				if (!PassesFilter(entry.level))
				{
					continue;
				}

				switch (entry.level)
				{
				case Royal::LogLevel::Warning:
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
					break;
				case Royal::LogLevel::Error:
				case Royal::LogLevel::Fatal:
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
					break;
				default:
					ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Text));
					break;
				}

				ImGui::TextUnformatted(entry.message.c_str());
				ImGui::PopStyleColor();
			}

			if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			{
				ImGui::SetScrollHereY(1.0f);
			}

			ImGui::EndChild();
		}
		ImGui::End();
	}
}