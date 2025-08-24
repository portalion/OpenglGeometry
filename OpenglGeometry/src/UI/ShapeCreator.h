#pragma once
#include "scene/Scene.h"
#include <imgui/imgui.h>
#include "archetypes/Archetypes.h"

namespace GUI
{
	inline std::vector<Entity> GetSelectedPoints(Ref<Scene> scene)
	{
		auto pointsView = scene->GetAllEntitiesWith<IsSelectedTag, NotificationComponent>();
		return std::vector<Entity>(pointsView.begin(), pointsView.end());
	}

	inline void DisplayCreationButtons(Ref<Scene> scene)
	{
		ImGui::Begin("Creation Menu##Creation menu");

		if (ImGui::Button("Create Torus##Creation menu"))
		{
			Archetypes::CreateTorus(scene.get());
		}

		if (ImGui::Button("Create Point##Creation menu"))
		{
			Archetypes::CreatePoint(scene.get());
		}

		if (ImGui::Button("Create Polyline##Creation menu"))
		{
			auto selectedPoints = GetSelectedPoints(scene);
			Archetypes::CreatePolyline(scene.get(), selectedPoints.begin(), selectedPoints.end());
		}

		if (ImGui::Button("Create Bezier C0##Creation menu"))
		{
			auto selectedPoints = GetSelectedPoints(scene);
			Archetypes::CreateBezierC0(scene.get(), selectedPoints.begin(), selectedPoints.end());
		}

		ImGui::End();
	}
}
