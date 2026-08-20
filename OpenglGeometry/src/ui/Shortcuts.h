#pragma once
#include <imgui/imgui.h>
#include "core/Base.h"
#include "scene/Scene.h"
#include "SceneActions.h"

namespace GUI
{
	inline void HandleShortcuts(Ref<Scene> scene)
	{
		if (ImGui::GetIO().WantTextInput)
		{
			return;
		}

		if (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_RouteGlobal))
		{
			DeleteSelected(scene);
		}
		if (ImGui::Shortcut(ImGuiKey_A, ImGuiInputFlags_RouteGlobal))
		{
			SelectAll(scene);
		}
		if (ImGui::Shortcut(ImGuiMod_Alt | ImGuiKey_A, ImGuiInputFlags_RouteGlobal))
		{
			DeselectAll(scene);
		}
		if (ImGui::Shortcut(ImGuiKey_F, ImGuiInputFlags_RouteGlobal))
		{
			FocusSelected(scene);
		}
	}
}
