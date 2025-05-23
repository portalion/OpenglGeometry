#pragma once
#include "core/RenderableOnScene.h"
#include <vector>
#include "Point.h"
#include <algorithm>

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
	inline void SwapPoints(std::weak_ptr<Point> point1, std::weak_ptr<Point> point2)
	{
		auto it1 = std::find_if(points.begin(), points.end(),
			[&point1](const std::weak_ptr<Point>& p) { return p.lock() == point1.lock(); });
		auto it2 = std::find_if(points.begin(), points.end(),
			[&point2](const std::weak_ptr<Point>& p) { return p.lock() == point2.lock(); });
		if (it1 != points.end() && it2 != points.end())
		{
			std::iter_swap(it1, it2);
		}
	}
};

