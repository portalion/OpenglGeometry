#pragma once
#include <vector>
#include <core/RenderableOnScene.h>
#include <objects/Point.h>
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
	
	std::shared_ptr<Point> GetPointByPosition(Algebra::Matrix4 VP, Algebra::Vector4 position) const;

	void DisplayUI();
	void Update();
	void Render();
	void AddPoint(std::shared_ptr<Point> point);
	void RemovePoint(std::shared_ptr<Point> point);
};

