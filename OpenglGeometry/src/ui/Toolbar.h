#pragma once
#include <string>
#include <imgui/imgui_internal.h>
#include "Style.h"
#include "model/UiState.h"

namespace GUI
{
	namespace Detail
	{
		inline void PivotButton(const char* label, PivotMode mode, PivotMode& current)
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

		inline void ActiveCurveChip(UiState& uiState)
		{
			ImGui::TextUnformatted("Active curve:");
			ImGui::SameLine();

			if (!uiState.activeCurveId)
			{
				ImGui::TextDisabled("none");
				return;
			}

			const ObjectRow* curve = uiState.Find(*uiState.activeCurveId);
			const std::string label = curve ? curve->name : ("#" + std::to_string(*uiState.activeCurveId));

			ImGui::BeginDisabled();
			ImGui::Button(label.c_str());
			ImGui::EndDisabled();

			ImGui::SameLine(0.f, 2.f);
			if (ImGui::SmallButton("x##ActiveCurveClear"))
			{
				uiState.activeCurveId.reset();
			}
		}
	}

	inline void DrawToolbar(UiState& uiState)
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const float height = ImGui::GetFrameHeight() + style.WindowPadding.y * 2.f;
		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;

		if (ImGui::BeginViewportSideBar("##Toolbar", ImGui::GetMainViewport(), ImGuiDir_Up, height, flags))
		{
			ImGui::TextUnformatted("Pivot");
			ImGui::SameLine();
			Detail::PivotButton("Origin##Pivot", PivotMode::Origin, uiState.pivot);
			ImGui::SameLine(0.f, Style::SegmentedButtonSpacing);
			Detail::PivotButton("Centre##Pivot", PivotMode::Centre, uiState.pivot);
			ImGui::SameLine(0.f, Style::SegmentedButtonSpacing);
			Detail::PivotButton("Cursor##Pivot", PivotMode::Cursor, uiState.pivot);

			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			ImGui::Checkbox("Grid", &uiState.showGrid);
			ImGui::SameLine();
			ImGui::Checkbox("Control nets", &uiState.showControlNets);
			ImGui::SameLine();
			ImGui::Checkbox("Virtual points", &uiState.showVirtualPoints);
			ImGui::SameLine();
			ImGui::Checkbox("Stereo", &uiState.stereo.enabled);

			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			Detail::ActiveCurveChip(uiState);

			ImGui::SameLine();
			ImGui::TextDisabled("(flags not yet wired to the scene)");
		}
		ImGui::End();
	}
}
