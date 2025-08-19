#pragma once
#include "scene/Scene.h"
#include <imgui/imgui.h>
#include "archetypes/Archetypes.h"

namespace GUI
{
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
			auto pointsView = scene->GetAllEntitiesWith<IsSelectedTag, NotificationComponent>();
			std::vector<Entity> pointsVector(pointsView.begin(), pointsView.end());
			Archetypes::CreatePolyline(scene.get(), pointsVector.begin(), pointsVector.end());
		}

		ImGui::End();
	}
}
