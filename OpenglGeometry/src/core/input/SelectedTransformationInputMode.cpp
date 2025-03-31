#include "SelectedTransformationInputMode.h"

void SelectedTransformationInputMode::HandleInput(const std::unordered_set<std::shared_ptr<RenderableOnScene>>& selectedItems)
{
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
	{
		ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
		Algebra::Vector4 direction = Algebra::Vector4(delta.x, -delta.y, 0, 0);
		if (direction.Length() > 0)
		{
			direction = direction.Normalize();
		}

		direction = direction * camera->GetViewMatrix();

		if (selectedItems.empty())
		{
			cursor->Move(direction);
		}
		else
		{
			for(auto& selected : selectedItems)
			{
				selected->Move(direction);
			}
		}
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
	}
}
