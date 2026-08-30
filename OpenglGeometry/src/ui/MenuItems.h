#pragma once
#include <imgui/imgui.h>
#include "core/Base.h"
#include "scene/Scene.h"
#include "SceneActions.h"
#include "Utils.h"
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
			RequestDialog(OpenSceneDialogTitle);
		}
		if (ImGui::MenuItem("Save", "Ctrl+S"))
		{
			RequestDialog(SaveSceneDialogTitle);
		}
		if (ImGui::MenuItem("Save as..."))
		{
			RequestDialog(SaveSceneDialogTitle);
		}
	}

	inline void DrawFileMenuItems(UiState& uiState, Ref<Scene> scene)
	{
		if (ImGui::MenuItem("New scene"))
		{
			scene->Clear();
			uiState.activeCurveId.reset();
			uiState.statusMessage = "new scene";
		}
		if (ImGui::MenuItem("Open...", "Ctrl+O"))
		{
			RequestDialog(OpenSceneDialogTitle);
		}
		if (ImGui::MenuItem("Save", "Ctrl+S"))
		{
			RequestDialog(SaveSceneDialogTitle);
		}
		if (ImGui::MenuItem("Save as..."))
		{
			RequestDialog(SaveSceneDialogTitle);
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
			RequestDialog(BezierSurfaceDialogTitle);
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

	inline void DrawCreateMenuItems(Ref<Scene> scene)
	{
		if (ImGui::MenuItem("Point"))
		{
			GUI::CreateShape(scene, ObjectType::Point);
		}
		if (ImGui::MenuItem("Torus"))
		{
			GUI::CreateShape(scene, ObjectType::Torus);
		}

		const bool hasPoints = !scene->GetAllEntitiesWith<IsSelectedTag, NotificationComponent>().empty();

		if (ImGui::MenuItem("Chain", nullptr, false, hasPoints))
		{
			GUI::CreateShape(scene, ObjectType::Chain);
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Bezier C0", nullptr, false, hasPoints))
		{
			GUI::CreateShape(scene, ObjectType::BezierC0);
		}
		if (ImGui::MenuItem("Bezier C2", nullptr, false, hasPoints))
		{
			GUI::CreateShape(scene, ObjectType::BezierC2);
		}
		if (ImGui::MenuItem("Interpolating C2", nullptr, false, hasPoints))
		{
			GUI::CreateShape(scene, ObjectType::InterpolatedC2);
		}
		if (!hasPoints && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("select one or more points first");
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Bezier surface..."))
		{
			RequestDialog(BezierSurfaceDialogTitle);
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

	inline void DrawEditMenuItems(Ref<Scene> scene, const UICallbacks& callbacks)
	{
		if (ImGui::MenuItem("Rename", "F2", false, GUI::CanRename(scene)))
		{
			callbacks.renameSelected();
		}
		if (ImGui::MenuItem("Delete selected", "Del", false, GUI::AnythingSelected(scene)))
		{
			GUI::DeleteSelected(scene);
		}
		if (ImGui::MenuItem("Collapse points", nullptr, false, GUI::CanCollapseSelection(scene)))
		{
			GUI::CollapseSelected(scene);
		}
	}

	inline void DrawSelectMenuItems(Ref<Scene> scene)
	{
		if (ImGui::MenuItem("Select all", "A"))
		{
			GUI::SelectAll(scene);
		}
		if (ImGui::MenuItem("Deselect all", "Alt+A", false, GUI::AnythingSelected(scene)))
		{
			GUI::DeselectAll(scene);
		}
	}

	inline void DrawViewDisplayItems(UiState& uiState, bool& showParameterSpace)
	{
		ImGui::MenuItem("Grid", nullptr, &uiState.showGrid);
		ImGui::MenuItem("Control nets", nullptr, &uiState.showControlNets);
		ImGui::MenuItem("Virtual points", nullptr, &uiState.showVirtualPoints);
		ImGui::MenuItem("Selection centre", nullptr, &uiState.showSelectionCentre);
		if (ImGui::MenuItem("Stereoscopy..."))
		{
			RequestDialog(StereoDialogTitle);
		}
		ImGui::MenuItem("Parameter space", nullptr, &showParameterSpace);
	}

	inline void DrawViewMenuItems(UiState& uiState, bool& showParameterSpace, bool& showImGuiDemo,
		bool& showLog, const UICallbacks& callbacks)
	{
		DrawViewDisplayItems(uiState, showParameterSpace);
		ImGui::MenuItem("Log", nullptr, &showLog);
		ImGui::Separator();
		ImGui::MenuItem("Dear ImGui demo", nullptr, &showImGuiDemo);
		if (ImGui::MenuItem("Reset layout"))
		{
			callbacks.resetLayout();
		}
	}

	inline void DrawAllDialogs(UiState& uiState)
	{
		FlushDialogRequest();
		DrawBezierSurfaceDialog(uiState);
		DrawSaveSceneDialog(uiState);
		DrawOpenSceneDialog(uiState);
		DrawStereoDialog(uiState);
		DrawAboutDialog();
	}

	inline void DrawAllDialogs(UiState& uiState, Ref<Scene> scene)
	{
		FlushDialogRequest();
		DrawBezierSurfaceDialog(uiState, scene);
		DrawSaveSceneDialog(uiState, scene);
		DrawOpenSceneDialog(uiState, scene);
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
			RequestDialog(OpenSceneDialogTitle);
		}
		if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
		{
			RequestDialog(SaveSceneDialogTitle);
		}
	}
}
