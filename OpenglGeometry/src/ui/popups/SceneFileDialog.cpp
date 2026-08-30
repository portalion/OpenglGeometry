#include "SceneFileDialog.h"

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

#include "serialization/SceneSerialization.h"

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

	bool DrawFileDialogBody(const char* title, std::string& path, const char* confirmLabel)
	{
		if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			return false;
		}

		DrawPathField(path);

		ImGui::TextDisabled("Gregory patches and intersection curves cannot be represented.");

		ImGui::Separator();

		bool confirmed = false;
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(confirmLabel))
		{
			confirmed = true;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
		return confirmed;
	}
}

void GUI::DrawSaveSceneDialog(UiState& state)
{
	static std::string path = "scenes/untitled.json";
	if (DrawFileDialogBody(SaveSceneDialogTitle, path, "Save"))
	{
		state.statusMessage = "saved to " + path + " (sandbox - no writer wired up)";
	}
}

void GUI::DrawOpenSceneDialog(UiState& state)
{
	static std::string path = "scenes/untitled.json";
	if (DrawFileDialogBody(OpenSceneDialogTitle, path, "Open"))
	{
		state.statusMessage = "would open " + path + " (sandbox - no loader wired up)";
	}
}

void GUI::DrawSaveSceneDialog(UiState& state, Ref<Scene> scene)
{
	static std::string path = "scenes/untitled.json";
	if (DrawFileDialogBody(SaveSceneDialogTitle, path, "Save"))
	{
		state.statusMessage = Serialization::SaveScene(*scene, path).message;
	}
}

void GUI::DrawOpenSceneDialog(UiState& state, Ref<Scene> scene)
{
	static std::string path = "scenes/untitled.json";
	if (DrawFileDialogBody(OpenSceneDialogTitle, path, "Open"))
	{
		state.statusMessage = Serialization::LoadScene(*scene, path).message;
	}
}
