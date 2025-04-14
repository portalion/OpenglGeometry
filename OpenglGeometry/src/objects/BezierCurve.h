#pragma once
#include "core/RenderableOnScene.h"
#include "Point.h"
#include <vector>
#include "Polyline.h"
#include <UI/SelectedShapes.h>

class BezierCurve : public RenderableOnScene
{
private:
	bool displayPolyline = false;
	std::vector<std::weak_ptr<Point>> points;

	inline std::string GetTypeName() const override { return "Bezier Curve"; }
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	bool DisplayParameters() override;

	Polyline polyline;
	bool HelperButton(ImGuiDir direction);
	SelectedShapes* selectedShapes = nullptr;
public:
	BezierCurve(std::vector<std::shared_ptr<Point>> points, SelectedShapes* shapes);
	inline void AddPoint(std::shared_ptr<Point> point)
	{
		polyline.AddPoint(point);
		points.push_back(point);
	}
	inline void RemovePoint(std::weak_ptr<Point> point)
	{
		points.erase(std::remove_if(points.begin(), points.end(),
			[&point](const std::weak_ptr<Point>& p) { return p.lock() == point.lock(); }), points.end());
	}
	inline void Render() const override
	{
		//RenderableOnScene::Render();
		if (displayPolyline)
		{
			polyline.Render();
		}
	}
	void Update() override
	{
		polyline.Update();
	}
};

