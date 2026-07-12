#include "Panels/LogPanel.h"

#include <imgui.h>

namespace Editor::Panels
{
	LogPanel& LogPanel::Get()
	{
		static LogPanel instance;
		return instance;
	}

	void LogPanel::AddLog(const std::string& message)
	{
		m_lines.push_back(message);
		if (m_lines.size() > kMaxLines)
		{
			m_lines.pop_front();
		}
	}

	void LogPanel::Draw(bool& open)
	{
		if (!open) return;

		if (ImGui::Begin("Log", &open))
		{
			if (ImGui::Button("Clear"))
			{
				m_lines.clear();
			}
			ImGui::SameLine();
			ImGui::Checkbox("Auto-scroll", &m_autoScroll);

			ImGui::Separator();

			ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
			
			for (const auto& line : m_lines)
			{
				ImGui::TextUnformatted(line.c_str());
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