#pragma once
#include <functional>
#include "utils/Initialization.h"
#include "SceneActions.h"
#include "Utils.h"

namespace GUI
{
	inline void DisabledMenuItem(const char* label, const char* shortcut = nullptr)
	{
		ImGui::BeginDisabled();
		ImGui::MenuItem(label, shortcut);
		ImGui::EndDisabled();
	}

	inline void DrawMenuBar(Ref<Scene> scene, bool& showImGuiDemo, const UICallbacks& callbacks)
	{
		if (!ImGui::BeginMainMenuBar())
		{
			return;
		}

		if (ImGui::BeginMenu("File"))
		{
			DisabledMenuItem("New scene");
			DisabledMenuItem("Open...", "Ctrl+O");
			DisabledMenuItem("Save", "Ctrl+S");
			DisabledMenuItem("Save as...");
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
			DisabledMenuItem("Point");
			DisabledMenuItem("Torus");
			DisabledMenuItem("Chain");
			ImGui::Separator();
			DisabledMenuItem("Bezier C0");
			DisabledMenuItem("Bezier C2");
			DisabledMenuItem("Interpolating C2");
			ImGui::Separator();
			DisabledMenuItem("Bezier surface...");
			DisabledMenuItem("Gregory patch");
			DisabledMenuItem("Intersection curve");
			ImGui::Separator();
			ImGui::TextDisabled("Shift + A opens the creation menu");
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
			DisabledMenuItem("Grid");
			DisabledMenuItem("Control nets");
			DisabledMenuItem("Virtual points");
			DisabledMenuItem("Stereoscopy...");
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
			DisabledMenuItem("About");
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}
