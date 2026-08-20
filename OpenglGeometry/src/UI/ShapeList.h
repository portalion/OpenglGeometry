#pragma once
#include <imgui/imgui.h>
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "Utils.h"
#include "SceneActions.h"

namespace GUI
{
	inline void DisplayShapeList(Ref<Scene> scene)
	{
		ImGui::Begin(ShapeListWindow);

		auto shapes = scene->GetAllEntitiesWith<NameComponent>();

		if (shapes.empty())
		{
			ImGui::Text("No shapes available.##Shape List");
			ImGui::End();
			return;
		}

		if (ImGui::Button("Select All##Shape List"))
		{
			SelectAll(scene);
		}

		ImGui::SameLine();

		if (ImGui::Button("Deselect All##Shape List"))
		{
			DeselectAll(scene);
		}

		if (ImGui::Button("Remove All Selected##Shape List"))
		{
			DeleteSelected(scene);
		}

		ImGui::SameLine();

		if (ImGui::Button("Focus Selected##Shape List"))
		{
			FocusSelected(scene);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Orbit around the selection (F)");
		}

		for (Entity entity : shapes)
		{	
			bool isSelected = entity.HasComponent<IsSelectedTag>();
			
			if (ImGui::Selectable(GenerateLabel(entity, entity.GetComponent<NameComponent>().name).c_str(), isSelected,
				ImGuiSelectableFlags_AllowDoubleClick))
			{
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					DeselectAll(scene);
					entity.AddTag<IsSelectedTag>();
					FocusSelected(scene);
				}
				else if (isSelected)
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
