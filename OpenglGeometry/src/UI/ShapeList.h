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

		for (auto entity : shapes)
		{	
			Entity e{ entity, scene.get() };
			bool isSelected = e.HasComponent<IsSelectedTag>();
			
			if (ImGui::Selectable(GenerateLabel(e, e.GetComponent<NameComponent>().name).c_str(), isSelected))
			{
				if (isSelected)
				{
					e.RemoveTag<IsSelectedTag>();
				}
				else
				{
					e.AddTag<IsSelectedTag>();
				}
			}
		}

		ImGui::End();
	}
}
