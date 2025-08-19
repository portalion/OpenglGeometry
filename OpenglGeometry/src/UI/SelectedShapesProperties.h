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
		for (Entity entity : selectedShapes)
		{
			ImGui::Text("Properties of %s", entity.GetComponent<NameComponent>().name.c_str());

			if (entity.HasComponent<PositionComponent>())
			{
				Algebra::Vector4 tmpPosition = entity.GetComponent<PositionComponent>().position;
				auto& position = entity.GetComponent<PositionComponent>().position;
				if (ImGui::DragFloat3(GenerateLabel(entity, "Position").c_str(), &tmpPosition.x, 0.1f))
				{
					position.Set(tmpPosition);
				}
			}

			if (entity.HasComponent<RotationComponent>())
			{
				auto& rotation = entity.GetComponent<RotationComponent>().rotation;
				ImGui::DragFloat3(GenerateLabel(entity, "Rotation").c_str(), &rotation.x, 0.1f);
			}

			if (entity.HasComponent<ScaleComponent>())
			{
				auto& scale = entity.GetComponent<ScaleComponent>().scale;
				ImGui::DragFloat3(GenerateLabel(entity, "Scale").c_str(), &scale.x, 0.1f);
			}
		}
		ImGui::End();
	}
}
