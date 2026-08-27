#pragma once
#include <imgui/imgui.h>
#include "core/Base.h"
#include "scene/Scene.h"
#include "Utils.h"
#include "MenuItems.h"
#include "model/UiState.h"
#include "core/DockSpace.h"

namespace GUI
{
	inline constexpr const char* ViewportContextMenuId = "ViewportContextMenu";

	inline void DrawViewportContextMenu(Ref<Scene> scene, const Dockspace& dockspace, UiState& uiState,
		bool& showParameterSpace, bool& showImGuiDemo, bool& showLog, const UICallbacks& callbacks)
	{
		ImGuiIO& io = ImGui::GetIO();

		ImVec2 rectMin;
		ImVec2 rectMax;
		if (dockspace.TryGetCentralNodeScreenRect(rectMin, rectMax))
		{
			const ImVec2 mouse = io.MousePos;
			const bool inViewport =
				mouse.x >= rectMin.x && mouse.x < rectMax.x &&
				mouse.y >= rectMin.y && mouse.y < rectMax.y;

			if (inViewport && io.KeyShift && !io.WantCaptureMouse
				&& ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup(ViewportContextMenuId);
			}
		}

		if (!ImGui::BeginPopup(ViewportContextMenuId))
		{
			return;
		}

		if (ImGui::BeginMenu("Create"))
		{
			DrawCreateMenuItems(scene);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			DrawEditMenuItems(scene, callbacks);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Select"))
		{
			DrawSelectMenuItems(scene);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			DrawViewMenuItems(uiState, showParameterSpace, showImGuiDemo, showLog, callbacks);
			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}
}
