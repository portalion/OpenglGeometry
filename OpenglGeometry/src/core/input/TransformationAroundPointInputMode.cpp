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
			middlePoint += selected->GetPositionComponent()->GetPosition();
		}
		middlePoint = middlePoint / selectedItems.size();
	}

	for (auto& selected : selectedItems)
	{
		selected->GetPositionComponent()->Move(direction);

		auto translation = selected->GetPositionComponent()->GetPosition() - middlePoint;
		auto translationAfterRotation = rotation.Rotate(translation);
		selected->GetPositionComponent()->SetPosition(translationAfterRotation + middlePoint);
		selected->GetRotationComponent()->Rotate(rotation.Conjugate());

		selected->GetPositionComponent()->SetPosition(middlePoint + translationAfterRotation * Algebra::Matrix4::DiagonalScaling(scale, scale, scale));
		selected->GetScaleComponent()->Scale(scale);
	}
}
