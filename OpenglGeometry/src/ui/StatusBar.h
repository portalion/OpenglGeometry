#pragma once
#include <ranges>
#include <imgui/imgui_internal.h>
#include "core/Globals.h"
#include "core/Viewport.h"

namespace GUI
{
	inline void DrawStatusBar(Ref<Scene> scene)
	{
		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;

		if (ImGui::BeginViewportSideBar("##StatusBar", ImGui::GetMainViewport(), ImGuiDir_Down, ImGui::GetFrameHeight() + 12, flags))
		{
			const auto objectCount = std::ranges::distance(scene->GetAllEntitiesWith<NameComponent>());
			const auto selectedCount = std::ranges::distance(scene->GetAllEntitiesWith<IsSelectedTag>());
			const ViewportData viewport = Globals::viewport.GetData();

			ImGui::Text("%.0f fps", ImGui::GetIO().Framerate);
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
			ImGui::Text("%lld objects, %lld selected", static_cast<long long>(objectCount), static_cast<long long>(selectedCount));
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
			ImGui::Text("viewport %d x %d", viewport.width, viewport.height);
		}
		ImGui::End();
	}
}