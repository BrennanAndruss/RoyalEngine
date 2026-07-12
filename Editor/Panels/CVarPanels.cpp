#include "Panels/CVarPanel.h"

#include "Engine/Core/CVar.h"

#include <imgui.h>

void Editor::Panels::DrawCVarPanel(bool& open)
{
	if (!open) return;

	if (ImGui::Begin("CVars", &open))
	{
		for (const auto& cvarPtr : Royal::CVarRegistry::GetAll())
		{
			Royal::CVarBase* base = cvarPtr.get();

			switch (base->GetType())
			{
			case Royal::CVarType::Bool:
			{
				auto* cvar = static_cast<Royal::CVarBool*>(base);
				ImGui::Checkbox(cvar->GetName().c_str(), cvar->GetValuePtr());
				break;
			}
			case Royal::CVarType::Int:
			{
				auto* cvar = static_cast<Royal::CVarInt*>(base);
				ImGui::SliderInt(cvar->GetName().c_str(), cvar->GetValuePtr(), cvar->GetMin(), cvar->GetMax());
				break;
			}
			case Royal::CVarType::Float:
			{
				auto* cvar = static_cast<Royal::CVarFloat*>(base);
				ImGui::SliderFloat(cvar->GetName().c_str(), cvar->GetValuePtr(), cvar->GetMin(), cvar->GetMax());
				break;
			}
			}
		}
	}
	ImGui::End();
}