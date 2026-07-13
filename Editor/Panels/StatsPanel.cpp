#include "Panels/StatsPanel.h"

#include "Engine/Core/Profiler.h"
#include "Engine/Core/Time.h"
#include "Engine/Platform/Window.h"

#include <imgui.h>

void Editor::Panels::DrawStatsPanel(bool &open, const Royal::Window& window,
	const std::vector<Royal::RHI::GPUSample>& gpuSamples)
{
	if (!open) return;

	if (ImGui::Begin("Stats", &open))
	{
		ImGui::Text("Frame time: %.3f ms", Royal::Time::GetDeltaTime() * 1000.0f);
		ImGui::Text("FPS: %.1f", 1.0f / Royal::Time::GetDeltaTime());
		ImGui::Separator();
		ImGui::Text("Window: %d x %d", window.GetWidth(), window.GetHeight());

		ImGui::Separator();
		ImGui::Text("CPU Scopes:");
		for (const auto& sample : Royal::Profiler::Get().GetLastFrameCPUSamples())
		{
			ImGui::Text("  %s: %.3f ms", sample.name.c_str(), sample.milliseconds);
		}

		ImGui::Separator();
		ImGui::Text("GPU Events:");
		for (const auto& sample : gpuSamples)
		{
			ImGui::Text("  %s: %.3f ms", sample.name.c_str(), sample.milliseconds);
		}		
	}
	ImGui::End();
}