#pragma once
#include "core/RenderableOnScene.h"
#include <vector>
#include "Point.h"


class Polyline : public RenderableOnScene
{
private:
	std::vector<std::weak_ptr<Point>> points;

	inline std::string GetTypeName() const override { return "Polyline"; }
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	bool DisplayParameters() override;
public:
	Polyline(std::vector<std::shared_ptr<Point>> points);
	inline void AddPoint(std::shared_ptr<Point> point)
	{
		points.push_back(point);
	}
	inline void RemovePoint(std::weak_ptr<Point> point)
	{
		points.erase(std::remove_if(points.begin(), points.end(),
			[&point](const std::weak_ptr<Point>& p) { return p.lock() == point.lock(); }), points.end());
	}
};

