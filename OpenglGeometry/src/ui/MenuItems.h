#pragma once
#include <imgui/imgui.h>
#include "model/UiState.h"
#include "popups/AboutDialog.h"
#include "popups/BezierSurfaceDialog.h"
#include "popups/SceneFileDialog.h"
#include "popups/StereoDialog.h"

namespace GUI
{
	inline void DisabledMenuItem(const char* label, const char* shortcut = nullptr)
	{
		ImGui::BeginDisabled();
		ImGui::MenuItem(label, shortcut);
		ImGui::EndDisabled();
	}

	inline void DrawFileMenuItems(UiState& uiState)
	{
		if (ImGui::MenuItem("New scene"))
		{
			uiState.objects.clear();
			uiState.activeCurveId.reset();
			uiState.statusMessage = "new scene (sandbox - the model was just cleared)";
		}
		if (ImGui::MenuItem("Open...", "Ctrl+O"))
		{
			ImGui::OpenPopup(OpenSceneDialogTitle);
		}
		if (ImGui::MenuItem("Save", "Ctrl+S"))
		{
			ImGui::OpenPopup(SaveSceneDialogTitle);
		}
		if (ImGui::MenuItem("Save as..."))
		{
			ImGui::OpenPopup(SaveSceneDialogTitle);
		}
	}

	inline void DrawCreateMenuItems(UiState& uiState)
	{
		if (ImGui::MenuItem("Point"))
		{
			uiState.AppendObject(ObjectType::Point, "Point");
		}
		if (ImGui::MenuItem("Torus"))
		{
			uiState.AppendObject(ObjectType::Torus, "Torus");
		}
		if (ImGui::MenuItem("Chain"))
		{
			uiState.AppendObject(ObjectType::Chain, "Chain");
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Bezier C0"))
		{
			uiState.AppendObject(ObjectType::BezierC0, "Bezier C0");
		}
		if (ImGui::MenuItem("Bezier C2"))
		{
			uiState.AppendObject(ObjectType::BezierC2, "Bezier C2");
		}
		if (ImGui::MenuItem("Interpolating C2"))
		{
			uiState.AppendObject(ObjectType::InterpolatedC2, "Interpolated");
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Bezier surface..."))
		{
			ImGui::OpenPopup(BezierSurfaceDialogTitle);
		}

		DisabledMenuItem("Gregory patch");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("needs a closed loop of 3 curve edges");
		}

		DisabledMenuItem("Intersection curve");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("needs 2 selected surfaces");
		}

		ImGui::Separator();
		ImGui::TextDisabled("Shift + A opens the creation menu");
	}

	inline void DrawViewDisplayItems(UiState& uiState, bool& showParameterSpace)
	{
		ImGui::MenuItem("Grid", nullptr, &uiState.showGrid);
		ImGui::MenuItem("Control nets", nullptr, &uiState.showControlNets);
		ImGui::MenuItem("Virtual points", nullptr, &uiState.showVirtualPoints);
		if (ImGui::MenuItem("Stereoscopy..."))
		{
			ImGui::OpenPopup(StereoDialogTitle);
		}
		ImGui::MenuItem("Parameter space", nullptr, &showParameterSpace);
	}

	inline void DrawAllDialogs(UiState& uiState)
	{
		DrawBezierSurfaceDialog(uiState);
		DrawSaveSceneDialog(uiState);
		DrawOpenSceneDialog(uiState);
		DrawStereoDialog(uiState);
		DrawAboutDialog();
	}

	inline void HandleFileShortcuts()
	{
		if (ImGui::GetIO().WantTextInput)
		{
			return;
		}

		if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal))
		{
			ImGui::OpenPopup(OpenSceneDialogTitle);
		}
		if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
		{
			ImGui::OpenPopup(SaveSceneDialogTitle);
		}
	}
}
