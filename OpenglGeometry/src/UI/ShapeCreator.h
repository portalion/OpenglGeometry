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

	inline void DisplayCreationButtons(Ref<Scene> scene, Algebra::Vector4 cursorPosition)
	{
		ImGui::Begin("Creation Menu##Creation menu");

		if (ImGui::Button("Create Torus##Creation menu"))
		{
			Archetypes::CreateTorus(scene.get());
		}

		if (ImGui::Button("Create Point##Creation menu"))
		{
			Archetypes::CreatePoint(scene.get(), { 0.f, 0.f, 0.f });
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

		if (ImGui::Button("Create Bezier C2##Creation menu"))
		{
			auto selectedPoints = GetSelectedPoints(scene);
			Archetypes::CreateBezierC2(scene.get(), selectedPoints.begin(), selectedPoints.end());
		}

		if (ImGui::Button("Create Interpolated Bezier##Creation menu"))
		{
			auto selectedPoints = GetSelectedPoints(scene);
			Archetypes::CreateInterpolatedBezier(scene.get(), selectedPoints.begin(), selectedPoints.end());
		}
#pragma region BezierSurfaceCreation
		{
			static Archetypes::BezierSurfaceCreationParameters bezierParams;
			ImGui::Separator();
			ImGui::DragInt("x patches##Creation menu",
				reinterpret_cast<int*>(&bezierParams.numberOfXPatches), 1, 1, 100);
			ImGui::DragInt("y patches##Creation menu", 
				reinterpret_cast<int*>(&bezierParams.numberOfYPatches), 1, 1, 100);
			ImGui::DragFloat("x size##Creation menu", 
				&bezierParams.sizeX, 1, 1, 100);
			ImGui::DragFloat("y size##Creation menu",
				&bezierParams.sizeY, 1, 1, 100);
			ImGui::Checkbox("Make cylinder##Creation menu", 
				&bezierParams.isCylinder);

			if (ImGui::Button("Create Bezier Surface##Creation menu"))
			{
				Archetypes::CreateBezierSurface(scene.get(), bezierParams);
			}
			ImGui::Separator();
		}
#pragma endregion 
		ImGui::End();
	}
}
