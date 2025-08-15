#pragma once
#include <imgui/imgui.h>
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "Utils.h"

namespace GUI
{
	inline void DisplaySelectedShapesProperties(Ref<Scene> scene)
	{
		ImGui::Begin("Selected Shapes Properties##Selected Shapes Properties");
		auto selectedShapes = scene->GetAllEntitiesWith<IsSelectedTag>();
		if (selectedShapes.empty())
		{
			ImGui::Text("No shapes selected.");
			ImGui::End();
			return;
		}
		for (auto entity : selectedShapes)
		{
			Entity e{ entity, scene.get() };
			ImGui::Text("Properties of %s", e.GetComponent<NameComponent>().name.c_str());

			if (e.HasComponent<PositionComponent>())
			{
				auto& position = e.GetComponent<PositionComponent>().position;
				ImGui::DragFloat3(GenerateLabel(e, "Position").c_str(), &position.x, 0.1f);

			}

			if (e.HasComponent<RotationComponent>())
			{
				auto& rotation = e.GetComponent<RotationComponent>().rotation;
				ImGui::DragFloat3(GenerateLabel(e, "Rotation").c_str(), &rotation.x, 0.1f);
			}

			if (e.HasComponent<ScaleComponent>())
			{
				auto& scale = e.GetComponent<ScaleComponent>().scale;
				ImGui::DragFloat3(GenerateLabel(e, "Scale").c_str(), &scale.x, 0.1f);	
			}
		}
		ImGui::End();
	}
}
