#pragma once
#include "InputMode.h"

class DefaultInputMode : public InputMode
{
private:
	Algebra::Vector4 draggingPoint;
	Algebra::Vector4 GetMousePoint(float x, float y);
public:
	DefaultInputMode(Window* window, Camera* camera);
	void HandleInput(const std::unordered_set<std::shared_ptr<RenderableOnScene>>& selectedItems) override;
};

