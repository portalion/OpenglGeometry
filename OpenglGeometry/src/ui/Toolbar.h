#pragma once
#include <imgui/imgui_internal.h>

namespace GUI
{
	inline void DrawToolbar()
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const float height = ImGui::GetFrameHeight() + style.WindowPadding.y * 2.f;
		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;

		if (ImGui::BeginViewportSideBar("##Toolbar", ImGui::GetMainViewport(), ImGuiDir_Up, height, flags))
		{
			ImGui::BeginDisabled();

			ImGui::TextUnformatted("Pivot");
			ImGui::SameLine();
			ImGui::Button("Origin");
			ImGui::SameLine(0.f, 1.f);
			ImGui::Button("Centre");
			ImGui::SameLine(0.f, 1.f);
			ImGui::Button("Cursor");

			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			bool on = true;
			bool off = false;
			ImGui::Checkbox("Grid", &on);
			ImGui::SameLine();
			ImGui::Checkbox("Control nets", &on);
			ImGui::SameLine();
			ImGui::Checkbox("Virtual points", &off);
			ImGui::SameLine();
			ImGui::Checkbox("Stereo", &off);

			ImGui::EndDisabled();

			ImGui::SameLine();
			ImGui::TextDisabled("(placeholders - phases 3, 5 and 7 wire these up)");
		}
		ImGui::End();
	}
}