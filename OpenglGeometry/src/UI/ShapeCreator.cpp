#include "ShapeCreator.h"
#include <objects/lines/Polyline.h>
#include <objects/Torus.h>
#include <objects/lines/BezierCurve.h>
#include <objects/lines/BezierCurveC2.h>
#include <objects/lines/InterpolatedBezierCurve.h>
#include <objects/surfaces/BezierSurface.h>
#include <objects/surfaces/BezierSurfaceC2.h>

std::shared_ptr<RenderableOnScene> ShapeCreator::GetShapeByType(ShapeEnum shape, ShapeList* shapeList) const
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
    case ShapeEnum::BezierSurface:
        return BezierSurface::Create(shapeList);
    case ShapeEnum::BezierSurfaceC2:
        return BezierSurfaceC2::Create(shapeList);
    }
    throw std::runtime_error("Invalid shape");
}

std::shared_ptr<RenderableOnScene> ShapeCreator::CreateShape(ShapeEnum shape, ShapeList* shapeList) const
{
    auto newShape = GetShapeByType(shape, shapeList);

    if (!newShape)
    {
        return nullptr;
    }

    newShape->InitName();
    newShape->Move(cursor->GetPosition());

    return newShape;
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
		 { ShapeEnum::BezierSurface, "Bezier Surface" },
		 { ShapeEnum::BezierSurfaceC2, "Bezier Surface C2" },
    };
    return shapeList;
}
