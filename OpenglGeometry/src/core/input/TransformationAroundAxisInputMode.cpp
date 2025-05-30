#include "TransformationAroundAxisInputMode.h"
#include <objects/Torus.h>

void TransformationAroundAxisInputMode::HandleInput(const std::unordered_set<std::shared_ptr<RenderableOnScene>>& selectedItems)
{
	auto direction = HandleTranslation();
	auto rotation = HandleRotation();
	auto scale = HandleScale();

	for (auto& selected : selectedItems)
	{
		bool isTorus = std::dynamic_pointer_cast<Torus>(selected) != nullptr;
		selected->Move(direction);

		auto translation = selected->GetPosition() - cursor->GetPosition();
		auto translationAfterRotation = rotation.Rotate(translation);
		selected->SetPosition(translationAfterRotation + cursor->GetPosition());
		selected->Rotate(rotation.Conjugate(), isTorus);

		selected->SetPosition(cursor->GetPosition()  + translationAfterRotation * Algebra::Matrix4::DiagonalScaling(scale, scale, scale));
		selected->Scale({ scale, scale, scale }, isTorus);
	}
}
