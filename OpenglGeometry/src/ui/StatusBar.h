#pragma once
#include <ranges>
#include <imgui/imgui_internal.h>
#include "core/Globals.h"
#include "core/Viewport.h"
#include "SceneActions.h"
#include "Style.h"
#include "model/UiState.h"

namespace GUI
{
	inline void DrawStatusBar(Ref<Scene> scene, const ViewportData& viewport, const UiState& uiState)
	{
		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
		const float height = ImGui::GetFrameHeight() + Style::StatusBarExtraHeight;

		if (ImGui::BeginViewportSideBar("##StatusBar", ImGui::GetMainViewport(), ImGuiDir_Down, height, flags))
		{
			const auto objectCount = std::ranges::distance(GetSceneObjects(scene));
			const auto selectedCount = std::ranges::distance(scene->GetAllEntitiesWith<IsSelectedTag>());

			ImGui::Text("%.0f fps", ImGui::GetIO().Framerate);
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
			ImGui::Text("%lld objects, %lld selected", static_cast<long long>(objectCount), static_cast<long long>(selectedCount));
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
			ImGui::Text("viewport %d x %d", viewport.width, viewport.height);
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
			ImGui::Text("pivot: %s", ToDisplayString(uiState.pivot));
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
			ImGui::Text("cursor %.3f, %.3f, %.3f",
				uiState.cursor.world.x, uiState.cursor.world.y, uiState.cursor.world.z);

			if (!uiState.statusMessage.empty())
			{
				ImGui::SameLine();
				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
				ImGui::SameLine();
				ImGui::TextDisabled("%s", uiState.statusMessage.c_str());
			}
		}
		ImGui::End();
	}
}