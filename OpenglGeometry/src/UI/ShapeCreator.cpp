#include "ShapeCreator.h"
#include <objects/Polyline.h>
#include <objects/Torus.h>
#include <objects/BezierCurve.h>
#include <objects/BezierCurveC2.h>
#include <objects/InterpolatedBezierCurve.h>

std::shared_ptr<RenderableOnScene> ShapeCreator::CreateShape(ShapeEnum shape) const
{
    switch (shape)
    {
    case ShapeEnum::Point:
        return std::make_shared<Point>();
    case ShapeEnum::Torus:
        return std::make_shared<Torus>();
    case ShapeEnum::Polyline:
        return std::make_shared<Polyline>(selectedShapes->GetSelectedWithType<Point>());
    case ShapeEnum::BezierCurveC0:
		return std::make_shared<BezierCurve>(selectedShapes->GetSelectedWithType<Point>(), selectedShapes);
    case ShapeEnum::BezierCurveC2:
		return std::make_shared<BezierCurveC2>(selectedShapes->GetSelectedWithType<Point>(), selectedShapes);
    case ShapeEnum::InterpolatedBezierCurve:
		return std::make_shared<InterpolatedBezierCurve>(selectedShapes->GetSelectedWithType<Point>(), selectedShapes);
    }
    throw std::runtime_error("Invalid shape");
}

const std::vector<std::pair<ShapeEnum, std::string>>& ShapeCreator::GetShapeList()
{
    static const std::vector<std::pair<ShapeEnum, std::string>> shapeList = {
         { ShapeEnum::Point, "Point" },
         { ShapeEnum::Torus, "Torus" },
		 { ShapeEnum::Polyline, "Polyline" },
		 { ShapeEnum::BezierCurveC0, "Bezier Curve C0" },
		 { ShapeEnum::BezierCurveC2, "Bezier Curve C2" },
		 { ShapeEnum::InterpolatedBezierCurve, "Interpolated Bezier Curve" },
    };
    return shapeList;
}
