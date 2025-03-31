#pragma once
#include "InputMode.h"

class TransformationAroundPointInputMode : public InputMode
{
public:
	using InputMode::InputMode;
	void HandleInput(const std::unordered_set<std::shared_ptr<RenderableOnScene>>& selectedItems) override;
};

