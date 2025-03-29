#pragma once
#include "core/RenderableOnScene.h"
#include <vector>
#include "Point.h"


class Polyline : public RenderableOnScene
{
private:
	std::vector<std::weak_ptr<Point>> points;

	inline std::string GetTypeName() const override { return "Polyline"; }
	RenderableOnSceneMesh GenerateMesh() override;
	bool DisplayParameters() override;
public:
	//Polyline(std::weak_ptr<Point> p)
	//{
	//	p.
	//}
	//Polyline(std::vector <std::weak_ptr<Point>> points);

	//AddPoint
};

