#pragma once
#include <imgui/imgui.h>
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "Utils.h"

namespace GUI
{
	inline void DisplayShapeList(Ref<Scene> scene)
	{
		ImGui::Begin("Shape List##Shape List");

		auto shapes = scene->GetAllEntitiesWith<NameComponent>();

		if (shapes.empty())
		{
			ImGui::Text("No shapes available.##Shape List");
			ImGui::End();
			return;
		}

		if (ImGui::Button("Select All##Shape List"))
		{
			for (Entity entity : shapes)
			{
				if (!entity.HasComponent<IsSelectedTag>())
				{
					entity.AddTag<IsSelectedTag>();
				}
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Deselect All##Shape List"))
		{
			for (Entity entity : shapes)
			{
				if (entity.HasComponent<IsSelectedTag>())
				{
					entity.RemoveTag<IsSelectedTag>();
				}
			}
		}

		for (Entity entity : shapes)
		{	
			bool isSelected = entity.HasComponent<IsSelectedTag>();
			
			if (ImGui::Selectable(GenerateLabel(entity, entity.GetComponent<NameComponent>().name).c_str(), isSelected))
			{
				if (isSelected)
				{
					entity.RemoveTag<IsSelectedTag>();
				}
				else
				{
					entity.AddTag<IsSelectedTag>();
				}
			}
		}

		ImGui::End();
	}
}
