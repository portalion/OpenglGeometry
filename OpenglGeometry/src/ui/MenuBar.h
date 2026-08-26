#pragma once
#include <functional>
#include "utils/Initialization.h"
#include "SceneActions.h"
#include "Utils.h"
#include "MenuItems.h"
#include "model/UiState.h"

namespace GUI
{
	inline void DrawMenuBar(Ref<Scene> scene, UiState& uiState, bool& showImGuiDemo, bool& showParameterSpace,
		const UICallbacks& callbacks)
	{
		if (!ImGui::BeginMainMenuBar())
		{
			return;
		}

		if (ImGui::BeginMenu("File"))
		{
			DrawFileMenuItems(uiState);
			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))
			{
				glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Rename", "F2", false, GUI::CanRename(scene)))
			{
				callbacks.renameSelected();
			}
			if (ImGui::MenuItem("Delete selected", "Del", false, GUI::AnythingSelected(scene)))
			{
				GUI::DeleteSelected(scene);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create"))
		{
			DrawCreateMenuItems(scene);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Select"))
		{
			if (ImGui::MenuItem("Select all", "A"))
			{
				GUI::SelectAll(scene);
			}
			if (ImGui::MenuItem("Deselect all", "Alt+A", false, GUI::AnythingSelected(scene)))
			{
				GUI::DeselectAll(scene);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			DrawViewDisplayItems(uiState, showParameterSpace);
			ImGui::Separator();
			ImGui::MenuItem("Dear ImGui demo", nullptr, &showImGuiDemo);
			if (ImGui::MenuItem("Reset layout"))
			{
				callbacks.resetLayout();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
			{
				ImGui::OpenPopup(AboutDialogTitle);
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}
