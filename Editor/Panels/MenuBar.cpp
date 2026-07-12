#include "Panels/MenuBar.h"

#include "Engine/Core/Application.h"

#include <imgui.h>

void Editor::Panels::DrawMenuBar(PanelState& state, Royal::Application& app)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app.RequestClose();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			ImGui::MenuItem("Stats", nullptr, &state.showStats);
			ImGui::MenuItem("Log", nullptr, &state.showLog);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}