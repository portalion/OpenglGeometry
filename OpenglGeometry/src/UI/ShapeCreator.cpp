#include "ShapeCreator.h"
#include <objects/Polyline.h>
#include <objects/Torus.h>

std::shared_ptr<RenderableOnScene> ShapeCreator::CreateShape(AvailableShapes shape) const
{
    //switch (shape)
    //{
    //case AvailableShapes::Point:
    //    return std::make_shared<Point>();
    //case AvailableShapes::Torus:
    //    return std::make_shared<Torus>();
    //case AvailableShapes::Polyline:
    //    return std::make_shared<Polyline>(selectedPoints);
    //}
    throw std::runtime_error("Invalid shape");
}