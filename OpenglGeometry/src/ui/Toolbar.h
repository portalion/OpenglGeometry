#pragma once
#include <imgui/imgui_internal.h>

namespace GUI
{
	enum class ToolbarPivotMode
	{
		Origin,
		Centre,
		Cursor,
	};

	namespace Detail
	{
		inline void PivotButton(const char* label, ToolbarPivotMode mode, ToolbarPivotMode& current)
		{
			const bool active = current == mode;

			if (active)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			}

			if (ImGui::Button(label))
			{
				current = mode;
			}

			if (active)
			{
				ImGui::PopStyleColor();
			}
		}
	}

	inline void DrawToolbar()
	{
		static ToolbarPivotMode s_Pivot = ToolbarPivotMode::Origin;
		static bool s_ShowGrid = true;
		static bool s_ShowControlNets = true;
		static bool s_ShowVirtualPoints = false;
		static bool s_Stereo = false;

		const ImGuiStyle& style = ImGui::GetStyle();
		const float height = ImGui::GetFrameHeight() + style.WindowPadding.y * 2.f;
		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;

		if (ImGui::BeginViewportSideBar("##Toolbar", ImGui::GetMainViewport(), ImGuiDir_Up, height, flags))
		{
			ImGui::TextUnformatted("Pivot");
			ImGui::SameLine();
			Detail::PivotButton("Origin##Pivot", ToolbarPivotMode::Origin, s_Pivot);
			ImGui::SameLine(0.f, 1.f);
			Detail::PivotButton("Centre##Pivot", ToolbarPivotMode::Centre, s_Pivot);
			ImGui::SameLine(0.f, 1.f);
			Detail::PivotButton("Cursor##Pivot", ToolbarPivotMode::Cursor, s_Pivot);

			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			ImGui::Checkbox("Grid", &s_ShowGrid);
			ImGui::SameLine();
			ImGui::Checkbox("Control nets", &s_ShowControlNets);
			ImGui::SameLine();
			ImGui::Checkbox("Virtual points", &s_ShowVirtualPoints);
			ImGui::SameLine();
			ImGui::Checkbox("Stereo", &s_Stereo);

			ImGui::SameLine();
			ImGui::TextDisabled("(not yet wired to the scene)");
		}
		ImGui::End();
	}
}
