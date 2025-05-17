#pragma once
#include <memory>
#include <core/RenderableOnScene.h>
#include "SelectedShapes.h"
#include <objects/AxisCursor.h>

enum class ShapeEnum
{
	Point = 0,
	Torus = 1,
	Polyline = 2,
	BezierCurveC0 = 3,
	BezierCurveC2 = 4,
	InterpolatedBezierCurve = 5,
	BezierSurface = 6,
	BezierSurfaceC2 = 7,
};

class ShapeCreator
{
private:
	SelectedShapes* selectedShapes;
	AxisCursor* cursor;
	std::shared_ptr<RenderableOnScene> GetShapeByType(ShapeEnum shape) const;
public:
	ShapeCreator(SelectedShapes* selectedShapes, AxisCursor* cursor) : selectedShapes{ selectedShapes }, cursor{ cursor } {}

	std::shared_ptr<RenderableOnScene> CreateShape(ShapeEnum shape) const;
	static const std::vector<std::pair<ShapeEnum, std::string>>& GetShapeList();
};