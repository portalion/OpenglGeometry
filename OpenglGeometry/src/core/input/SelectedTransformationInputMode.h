#pragma once
#include "InputMode.h"

class SelectedTransformationInputMode : public InputMode
{
private:
	static const std::vector<std::pair<Algebra::Vector4, ImGuiKey>> rotationMapping;
	Algebra::Quaternion RotateAlongAxis(Algebra::Vector4 axis);
protected:
	Algebra::Vector4 HandleTranslation();
	Algebra::Quaternion HandleRotation();
	float HandleScale();
public:
	using InputMode::InputMode;
	virtual ~SelectedTransformationInputMode() = default;
	virtual void HandleInput(const std::unordered_set<std::shared_ptr<RenderableOnScene>>& selectedItems) override;
};

