#pragma once
#include <memory>
#include <core/RenderableOnScene.h>

enum class AvailableShapes
{
	Point = 0,
	Torus = 1,
	Polyline = 2
};

class ShapeCreator
{
private:
	std::shared_ptr<RenderableOnScene> CreateShape(AvailableShapes shape) const;
public:
};