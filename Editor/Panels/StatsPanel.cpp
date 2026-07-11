#include "Panels/StatsPanel.h"

#include <imgui.h>

void Editor::Panels::DrawStatsPanel()
{
	if (ImGui::Begin("Stats"))
	{
		ImGui::Text("Frame time: %.2f ms", 16.6f);
	}
	ImGui::End();
}