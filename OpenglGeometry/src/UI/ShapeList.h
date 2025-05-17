#pragma once
#include <vector>
#include <core/RenderableOnScene.h>
#include <memory>
#include "SelectedShapes.h"
#include "ShapeCreator.h"

class ShapeList
{
private:
	SelectedShapes* selectedShapes;
	ShapeCreator shapeCreator;
	std::vector<std::shared_ptr<RenderableOnScene>> shapes;
	std::shared_ptr<Shader> defaultShader;

	void CreateShapeButton();
public:
	ShapeList(AxisCursor* axis, SelectedShapes* selectedShapes);

	void DisplayUI();
	void Update();
	void Render();
};

