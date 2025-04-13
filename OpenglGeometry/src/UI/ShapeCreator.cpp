#include "ShapeCreator.h"
#include <objects/Polyline.h>
#include <objects/Torus.h>

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
    }
    throw std::runtime_error("Invalid shape");
}

const std::vector<std::pair<ShapeEnum, std::string>>& ShapeCreator::GetShapeList()
{
    static const std::vector<std::pair<ShapeEnum, std::string>> shapeList = {
         { ShapeEnum::Point, "Point" },
         { ShapeEnum::Torus, "Torus" },
         { ShapeEnum::Polyline, "Polyline" },
    };
    return shapeList;
}
