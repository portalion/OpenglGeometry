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
		bool& showLog, const UICallbacks& callbacks)
	{
		if (!ImGui::BeginMainMenuBar())
		{
			return;
		}

		if (ImGui::BeginMenu("File"))
		{
			DrawFileMenuItems(uiState, scene);
			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))
			{
				glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			DrawEditMenuItems(scene, callbacks);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create"))
		{
			DrawCreateMenuItems(scene);
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

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
			{
				RequestDialog(AboutDialogTitle);
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}
