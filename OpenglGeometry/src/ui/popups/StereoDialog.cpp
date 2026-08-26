#include "StereoDialog.h"

#include <algorithm>
#include <imgui/imgui.h>

#include "ui/Widgets.h"

namespace
{
	void DrawAnaglyphPreview(const StereoSettings& stereo)
	{
		ImGui::BeginChild("##AnaglyphPreview", ImVec2(176.f, 94.f), ImGuiChildFlags_Border);

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImVec2 origin = ImGui::GetWindowPos();
		const ImVec2 size = ImGui::GetWindowSize();
		const ImVec2 center(origin.x + size.x * 0.5f, origin.y + size.y * 0.5f);

		const float offset = std::clamp(stereo.eyeDistance * 200.f, 4.f, size.x * 0.35f);
		const ImVec2 radius(size.x * 0.28f, size.y * 0.32f);

		const ImU32 leftColor = ImGui::ColorConvertFloat4ToU32(
			ImVec4(stereo.leftEyeColor.x, stereo.leftEyeColor.y, stereo.leftEyeColor.z, 0.75f));
		const ImU32 rightColor = ImGui::ColorConvertFloat4ToU32(
			ImVec4(stereo.rightEyeColor.x, stereo.rightEyeColor.y, stereo.rightEyeColor.z, 0.75f));

		drawList->AddEllipse(ImVec2(center.x - offset * 0.5f, center.y), radius, leftColor, 0.f, 0, 1.6f);
		drawList->AddEllipse(ImVec2(center.x + offset * 0.5f, center.y), radius, rightColor, 0.f, 0, 1.6f);

		ImGui::EndChild();
	}
}

void GUI::DrawStereoDialog(UiState& state)
{
	if (!ImGui::BeginPopupModal(StereoDialogTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	StereoSettings& stereo = state.stereo;

	ImGui::Checkbox("Enabled", &stereo.enabled);

	if (BeginPropertyTable("##Stereo"))
	{
		PropertyRow("Eye distance", stereo.eyeDistance, 0.005f);
		PropertyRow("Convergence", stereo.convergence, 0.1f);
		EndPropertyTable();
	}

	ImGui::TextUnformatted("Eye colours");
	ImGui::SameLine();
	ImGui::ColorEdit3("Left##EyeColor", &stereo.leftEyeColor.x, ImGuiColorEditFlags_NoInputs);
	ImGui::SameLine();
	ImGui::ColorEdit3("Right##EyeColor", &stereo.rightEyeColor.x, ImGuiColorEditFlags_NoInputs);
	ImGui::SameLine();
	ImGui::TextDisabled("left / right");

	ImGui::Spacing();
	DrawAnaglyphPreview(stereo);

	ImGui::Separator();

	if (ImGui::Button("Close"))
	{
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}
