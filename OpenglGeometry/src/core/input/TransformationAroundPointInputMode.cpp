#include "TransformationAroundPointInputMode.h"

void TransformationAroundPointInputMode::HandleInput(const std::unordered_set<std::shared_ptr<RenderableOnScene>>& selectedItems)
{
	auto direction = HandleTranslation();
	auto rotation = HandleRotation();
	auto scale = HandleScale();

	Algebra::Vector4 middlePoint;
	if (!selectedItems.empty())
	{
		for (auto& selected : selectedItems)
		{
			middlePoint += selected->GetPosition();
		}
		middlePoint = middlePoint / selectedItems.size();
	}

	for (auto& selected : selectedItems)
	{
		selected->Move(direction);

		auto translation = selected->GetPosition() - middlePoint;
		auto translationAfterRotation = rotation.Rotate(translation);
		selected->SetPosition(translationAfterRotation + middlePoint);
		selected->Rotate(rotation.Conjugate());

		selected->SetPosition(middlePoint + translationAfterRotation * Algebra::Matrix4::DiagonalScaling(scale, scale, scale));
		selected->Scale({ scale, scale, scale });
	}
}
