#include "SelectedTransformationInputMode.h"
#include <core/Globals.h>

const std::vector<std::pair<Algebra::Vector4, ImGuiKey>> SelectedTransformationInputMode::rotationMapping =
{
	std::make_pair<Algebra::Vector4, ImGuiKey>(Algebra::Vector4(1, 0, 0, 0), ImGuiKey_X),
	std::make_pair<Algebra::Vector4, ImGuiKey>(Algebra::Vector4(0, 1, 0, 0), ImGuiKey_Y),
	std::make_pair<Algebra::Vector4, ImGuiKey>(Algebra::Vector4(0, 0, 1, 0), ImGuiKey_Z)
};

Algebra::Vector4 SelectedTransformationInputMode::HandleTranslation()
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

		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
		return direction;
	}
	return Algebra::Vector4();
}

Algebra::Quaternion SelectedTransformationInputMode::RotateAlongAxis(Algebra::Vector4 axis)
{
	Algebra::Quaternion rotation;
	ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
	float rollDelta = delta.x / Globals::startingSceneWidth * 3.f;

	Algebra::Vector4 forward = rotation.Rotate(axis);

	Algebra::Quaternion rollQuat = Algebra::Quaternion::CreateFromAxisAngle(forward, rollDelta);
	rotation = (rollQuat * rotation).Normalize();
	ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
	return rotation;
}

Algebra::Quaternion SelectedTransformationInputMode::HandleRotation()
{
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
	{
		for (const auto& inputMap : rotationMapping)
		{
			if (ImGui::IsKeyDown(inputMap.second))
			{
				return RotateAlongAxis(inputMap.first);
			}
		}
	}

	return Algebra::Quaternion();
}

float SelectedTransformationInputMode::HandleScale()
{
	if (ImGui::GetIO().MouseWheel != 0.f)
	{
		return ImGui::GetIO().MouseWheel * 0.1f + 1.f;
	}
	return 1.f;
}

void SelectedTransformationInputMode::HandleInput(const std::unordered_set<std::shared_ptr<RenderableOnScene>>& selectedItems)
{
	auto direction = HandleTranslation();
	auto rotation = HandleRotation();
	auto scale = HandleScale();

	if (selectedItems.empty())
	{
		cursor->Move(direction);
	}
	else
	{
		for (auto& selected : selectedItems)
		{
			selected->Move(direction);
			selected->Rotate(rotation);
			selected->Scale({ scale, scale, scale });
		}
	}
}
