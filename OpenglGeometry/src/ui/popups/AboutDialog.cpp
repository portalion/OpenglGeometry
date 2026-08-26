#include "AboutDialog.h"

#include <imgui/imgui.h>

void GUI::DrawAboutDialog()
{
	if (!ImGui::BeginPopupModal(AboutDialogTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	ImGui::TextUnformatted("Geometry Editor");
	ImGui::TextDisabled("OpenglGeometry");

	ImGui::Separator();

	ImGui::Text("Dear ImGui %s", ImGui::GetVersion());
	ImGui::TextDisabled("docking branch");

	ImGui::Spacing();
	ImGui::TextWrapped(
		"A Bezier and B-spline modelling tool built around a small ECS scene and an OpenGL renderer.");

	ImGui::Separator();

	if (ImGui::Button("Close"))
	{
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}
