#include "Panels/StatsPanel.h"

#include "Engine/Platform/Window.h"

#include <imgui.h>

void Editor::Panels::DrawStatsPanel(bool &open, const Royal::Window& window)
{
	if (!open) return;

	if (ImGui::Begin("Stats", &open))
	{
		ImGui::Text("Frame time: %.2f ms", 0.0f);
		ImGui::Text("FPS: %.1f", 0.0f);
		ImGui::Separator();
		ImGui::Text("GPU: %s", "unknown");
		ImGui::Text("Window: %d x %d", window.GetWidth(), window.GetHeight());
	}
	ImGui::End();
}