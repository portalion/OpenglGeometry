#include "TransformationAroundAxisInputMode.h"

void TransformationAroundAxisInputMode::HandleInput(const std::unordered_set<std::shared_ptr<RenderableOnScene>>& selectedItems)
{
	auto direction = HandleTranslation();
	auto rotation = HandleRotation();
	auto scale = HandleScale();

	for (auto& selected : selectedItems)
	{
		selected->GetPositionComponent()->Move(direction);

		auto translation = selected->GetPositionComponent()->GetPosition() - cursor->GetPositionComponent()->GetPosition();
		auto translationAfterRotation = rotation.Rotate(translation);
		selected->GetPositionComponent()->SetPosition(translationAfterRotation + cursor->GetPositionComponent()->GetPosition());
		selected->GetRotationComponent()->Rotate(rotation.Conjugate());

		selected->GetPositionComponent()->SetPosition(cursor->GetPositionComponent()->GetPosition()  + translationAfterRotation * Algebra::Matrix4::DiagonalScaling(scale, scale, scale));
		selected->GetScaleComponent()->Scale(scale);
	}
}
