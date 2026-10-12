#pragma once
#include <ranges>
#include <imgui/imgui_internal.h>
#include "core/Globals.h"
#include "core/Log.h"
#include "core/Viewport.h"
#include "SceneActions.h"
#include "Style.h"

namespace GUI
{
	inline void DrawStatusBar(Ref<Scene> scene, const ViewportData& viewport)
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
			for (Entity cursor : scene->GetAllEntitiesWith<CursorTag, PositionComponent>())
			{
				const Algebra::Vector4 world = cursor.GetComponent<PositionComponent>().position;
				ImGui::Text("cursor %.3f, %.3f, %.3f", world.x, world.y, world.z);
				break;
			}

			const auto& entries = Logger::Entries();
			if (!entries.empty())
			{
				ImGui::SameLine();
				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
				ImGui::SameLine();
				ImGui::TextDisabled("%s", entries.back().message.c_str());
			}
		}
		ImGui::End();
	}
}