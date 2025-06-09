#include "ShapeCreator.h"
#include <objects/lines/Polyline.h>
#include <objects/Torus.h>
#include <objects/lines/BezierCurve.h>
#include <objects/lines/BezierCurveC2.h>
#include <objects/lines/InterpolatedBezierCurve.h>
#include <objects/surfaces/BezierSurface.h>
#include <objects/surfaces/BezierSurfaceC2.h>

std::vector<GraphTriangle> ShapeCreator::GetTriangles(const Graph& graph) const
{
    std::vector<GraphTriangle> triangles;
    const auto& neighbours = graph.neighbours;

    size_t n = graph.vertices.size();

    for (size_t i = 0; i < n; ++i) {
        for (unsigned int j : neighbours[i]) 
        {
            if (j <= i) continue;  

            for (unsigned int k : neighbours[j])
            {
				auto a = graph.vertices[i], b = graph.vertices[j], c = graph.vertices[k];

                if (k <= j || k == i) continue;
                GraphTriangle tri{
                        graph.vertices[i],
                        graph.vertices[j],
                        graph.vertices[k]
                };

                if (a.vertex1 == c.vertex2 && a.vertex2 == b.vertex1 && b.vertex2 == c.vertex1) triangles.push_back(tri);
                else if (a.vertex1 == c.vertex1 && a.vertex2 == b.vertex1 && b.vertex2 == c.vertex2) triangles.push_back(tri);
                else if (a.vertex1 == c.vertex1 && a.vertex2 == b.vertex2 && b.vertex1 == c.vertex2) triangles.push_back(tri);
                else if (a.vertex1 == c.vertex2 && a.vertex2 == b.vertex2 && b.vertex1 == c.vertex1) triangles.push_back(tri);

                else if (a.vertex2 == c.vertex2 && a.vertex1 == b.vertex1 && b.vertex2 == c.vertex1) triangles.push_back(tri);
                else if (a.vertex2 == c.vertex1 && a.vertex1 == b.vertex1 && b.vertex2 == c.vertex2) triangles.push_back(tri);
                else if (a.vertex2 == c.vertex1 && a.vertex1 == b.vertex2 && b.vertex1 == c.vertex2) triangles.push_back(tri);
                else if (a.vertex2 == c.vertex2 && a.vertex1 == b.vertex2 && b.vertex1 == c.vertex1) triangles.push_back(tri);
            }
        }
    }

    return triangles;
}

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
		shapeList->StartCreationMode(false);
        return nullptr;
    case ShapeEnum::BezierSurfaceC2:
        shapeList->StartCreationMode(true);
        return nullptr;
    case ShapeEnum::GregoryPatch:
        auto graph = shapeList->GetSelectedShapes()->GetGraph();
        auto triangles = GetTriangles(graph);
        return nullptr;
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

std::shared_ptr<RenderableOnScene> ShapeCreator::DeserializeShape(const json& j, ShapeList* list) const
{
	auto shapeTypeStr = j["objectType"].get<std::string>();
    if (shapeTypeStr == "torus")
    {
		return Torus::Deserialize(j);
	}
	else if (shapeTypeStr == "chain")
	{
		return Polyline::Deserialize(j, list);
	}
    else if (shapeTypeStr == "interpolatedC2")
    {
        return InterpolatedBezierCurve::Deserialize(j, list);
    } 
    else if (shapeTypeStr == "bezierC2")
    {
        return BezierCurveC2::Deserialize(j, list);
    }
    else if (shapeTypeStr == "bezierC0")
	{
		return BezierCurve::Deserialize(j, list);
	}
	else if (shapeTypeStr == "bezierSurfaceC0")
	{
		return BezierSurface::Deserialize(j, list);
	}
	else if (shapeTypeStr == "bezierSurfaceC2")
	{
	    return BezierSurfaceC2::Deserialize(j, list);
	}

    return nullptr;
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
         { ShapeEnum::GregoryPatch, "Gregory patch"}
    };
    return shapeList;
}
