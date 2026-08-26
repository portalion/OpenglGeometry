#include "SceneFileDialog.h"

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

namespace
{
	void DrawPathField(std::string& path)
	{
		ImGui::TextUnformatted("Path");
		ImGui::SameLine();

		ImGui::SetNextItemWidth(320.f);
		ImGui::InputText("##Path", &path);

		ImGui::SameLine();

		ImGui::BeginDisabled();
		ImGui::Button("Browse...");
		ImGui::EndDisabled();

		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("no file dialog is vendored yet - type the path directly");
		}
	}
}

void GUI::DrawSaveSceneDialog(UiState& state)
{
	if (!ImGui::BeginPopupModal(SaveSceneDialogTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	static std::string path;

	if (ImGui::IsWindowAppearing())
	{
		path = "scenes/untitled.json";
	}

	DrawPathField(path);

	ImGui::Spacing();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.55f, 0.2f, 1.f));
	ImGui::TextWrapped("Gregory patches and intersection curves will be skipped.");
	ImGui::PopStyleColor();
	ImGui::TextDisabled("the format has no representation for them");

	ImGui::Separator();

	if (ImGui::Button("Cancel"))
	{
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Save"))
	{
		state.statusMessage = "saved to " + path + " (sandbox - no writer wired up)";
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void GUI::DrawOpenSceneDialog(UiState& state)
{
	if (!ImGui::BeginPopupModal(OpenSceneDialogTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	static std::string path;

	if (ImGui::IsWindowAppearing())
	{
		path = "scenes/untitled.json";
	}

	DrawPathField(path);

	ImGui::Separator();

	if (ImGui::Button("Cancel"))
	{
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Open"))
	{
		state.statusMessage = "would open " + path + " (sandbox - no loader wired up)";
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}
