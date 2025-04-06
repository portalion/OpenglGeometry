#pragma once
#include "SelectedTransformationInputMode.h"

class TransformationAroundAxisInputMode : public SelectedTransformationInputMode
{
public:
	using SelectedTransformationInputMode::SelectedTransformationInputMode;
	void HandleInput(const std::unordered_set<std::shared_ptr<RenderableOnScene>>& selectedItems) override;
};

